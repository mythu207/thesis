package com.example.testapp;

import static android.content.ContentValues.TAG;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Switch;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Set;

public class GattActivity extends AppCompatActivity {
    String TAG = "BLEGattActivity";
    private BluetoothLeService bluetoothService;
    boolean connected;
    String deviceAddress;
    Button btnSendDiscoverCommand;


    Switch switchNotify;
    ArrayList mGattCharacteristics;

    ArrayAdapter<String> threadDeviceAdapter;
    public ArrayList<String> threadDeviceAddressList;
    public int numberOfThreadDevice = 0;

    private final String LIST_NAME = "NAME";
    private final String LIST_UUID = "UUID";
    static boolean notifyState = false;
    private final static String CccdUUID = "00002902-0000-1000-8000-00805f9b34fb";

    private ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            bluetoothService = ((BluetoothLeService.LocalBinder) service).getService();


            if (bluetoothService != null) {
                // call functions on service to check connection and connect to devices
                if (!bluetoothService.initialize()) {
                    Log.e(TAG, "Unable to initialize Bluetooth");
                    finish();
                }
                bluetoothService.connect(deviceAddress);
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            bluetoothService = null;
        }
    };

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_gatt);
        btnSendDiscoverCommand = findViewById(R.id.btnSendDiscoverCommand);
        switchNotify = findViewById(R.id.notifySwitch);

        Intent intent = getIntent();
        if(null != intent){
            deviceAddress = intent.getStringExtra("deviceAddress");
            Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);
            bindService(gattServiceIntent, serviceConnection, Context.BIND_AUTO_CREATE);
            registerReceiver(gattUpdateReceiver, makeGattUpdateIntentFilter());
        }
        switchNotify.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
                bluetoothService.writeNotification(isChecked);
                notifyState = isChecked;
                if(isChecked){
                    Log.i(TAG, "isChecked - Notify ON");
                }
                else{
                    Log.i(TAG, "Notify OFF");
                }
            }
        });


        btnSendDiscoverCommand.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //bluetoothService.writeNotification(true);
                bluetoothService.writeRxCharacteristic("discover");
                threadDeviceAdapter = new ArrayAdapter<String>(GattActivity.this, android.R.layout.select_dialog_singlechoice);
                threadDeviceAddressList = new ArrayList<String>();

                AlertDialog.Builder builderSingle = new AlertDialog.Builder(GattActivity.this);
                builderSingle.setTitle("Thread device list: ");
                builderSingle.setNegativeButton("cancel", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });

                builderSingle.setAdapter(threadDeviceAdapter, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Intent intent = new Intent(GattActivity.this, ThreadControlActivity.class);
                        intent.putExtra("deviceAddress", threadDeviceAddressList.get(which));
                        startActivity(intent);
                    }
                });
                builderSingle.show();
            }
        });



    }
    private final BroadcastReceiver gattUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (BluetoothLeService.ACTION_GATT_CONNECTED.equals(action)) {
                connected = true;
                updateConnectionState("Connected");
            } else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
                connected = false;
                updateConnectionState("Disconnected");
            } else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                // Show all the supported services and characteristics on the user interface.
                updateConnectionState("ACTION_GATT_SERVICES_DISCOVERED");
                displayGattServices(bluetoothService.getSupportedGattServices());
                Log.d(TAG, "readCharacteristics: ");
                bluetoothService.readCharacteristic(mGattCharacteristics);
            } else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)){
                updateConnectionState("ACTION_DATA_AVAILABLE");
                if(intent.getExtras().containsKey(BluetoothLeService.EXTRA_ADDRESS)){
                    Log.i(TAG, intent.getStringExtra(BluetoothLeService.EXTRA_ADDRESS));
                    threadDeviceAddressList.add(intent.getStringExtra(BluetoothLeService.EXTRA_ADDRESS));
                    threadDeviceAdapter.add(intent.getStringExtra(BluetoothLeService.EXTRA_ADDRESS));
                }
                else if(intent.getExtras().containsKey(BluetoothLeService.EXTRA_NUMBER_OF_DEVICES)){
                    Log.i(TAG, "There is/are " + intent.getIntExtra(BluetoothLeService.EXTRA_NUMBER_OF_DEVICES,-1) +" Thread device(s) found");
                }
            }
            else {
                updateConnectionState(action);
                Log.e("Thu", "gattUpdateReceiver: " + intent.getStringExtra(BluetoothLeService.EXTRA_DATA));

            }
        }
    };

    @Override
    protected void onResume() {
        super.onResume();

        registerReceiver(gattUpdateReceiver, makeGattUpdateIntentFilter());
        if (bluetoothService != null) {
            final boolean result = bluetoothService.connect(deviceAddress);
            Log.d(TAG, "Connect request result=" + result);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver(gattUpdateReceiver);
    }

    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
        return intentFilter;
    }
    private void updateConnectionState(String connectionState){
        Log.i(TAG, connectionState);
    }
    private void displayGattServices(List<BluetoothGattService> gattServices) {
        if (gattServices == null) return;
        String uuid = null;
        String unknownServiceString = getResources().
                getString(R.string.unknown_service);
        String unknownCharaString = getResources().
                getString(R.string.unknown_characteristic);
        ArrayList<HashMap<String, String>> gattServiceData =
                new ArrayList<HashMap<String, String>>();
        ArrayList<ArrayList<HashMap<String, String>>> gattCharacteristicData
                = new ArrayList<ArrayList<HashMap<String, String>>>();
        mGattCharacteristics =
                new ArrayList<ArrayList<BluetoothGattCharacteristic>>();

        // Loops through available GATT Services.
        for (BluetoothGattService gattService : gattServices) {
            HashMap<String, String> currentServiceData =
                    new HashMap<String, String>();
            uuid = gattService.getUuid().toString();
            currentServiceData.put(
                    LIST_NAME, SampleGattAttributes.
                            lookup(uuid, unknownServiceString));
            currentServiceData.put(LIST_UUID, uuid);
            gattServiceData.add(currentServiceData);

            ArrayList<HashMap<String, String>> gattCharacteristicGroupData =
                    new ArrayList<HashMap<String, String>>();
            List<BluetoothGattCharacteristic> gattCharacteristics =
                    gattService.getCharacteristics();
            ArrayList<BluetoothGattCharacteristic> charas =
                    new ArrayList<BluetoothGattCharacteristic>();
            // Loops through available Characteristics.
            for (BluetoothGattCharacteristic gattCharacteristic :
                    gattCharacteristics) {
                charas.add(gattCharacteristic);
                HashMap<String, String> currentCharaData =
                        new HashMap<String, String>();
                uuid = gattCharacteristic.getUuid().toString();
                currentCharaData.put(
                        LIST_NAME, SampleGattAttributes.lookup(uuid,
                                unknownCharaString));
                currentCharaData.put(LIST_UUID, uuid);
                gattCharacteristicGroupData.add(currentCharaData);
            }
            mGattCharacteristics.add(charas);
            gattCharacteristicData.add(gattCharacteristicGroupData);
        }
        System.out.println("gattCharacteristicsData");
        for( ArrayList<HashMap<String, String>> arrSubList : gattCharacteristicData){
            for(HashMap<String, String> characteristicData : arrSubList){
                Set<String> keyset = characteristicData.keySet();
                for(String key : keyset){
                    System.out.println(key + ": " + characteristicData.get(key));
                }
            }
        }
    }
}



