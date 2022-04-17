package com.example.testapp;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelUuid;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {
    BluetoothAdapter mBluetoothAdapter;
    BluetoothLeScanner bluetoothLeScanner;
    private boolean scanning;

    private Handler handler = new Handler(Looper.getMainLooper());
    private static final long SCAN_PERIOD = 10000;
    Button btnStartBluetooth, btnStartScanning;
    ArrayAdapter<String> arrayAdapter;
    ArrayList<String> ListScanDevice;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        btnStartBluetooth = findViewById(R.id.btnStartBluetooth);
        btnStartScanning = findViewById(R.id.btnStartScanning);

        btnStartBluetooth.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                if (!mBluetoothAdapter.isEnabled()) {
                    if (ActivityCompat.checkSelfPermission(MainActivity.this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                        ActivityCompat.requestPermissions(MainActivity.this,
                                new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 1);
                    }
                    mBluetoothAdapter.enable();
                    DisplayToastMsg("Bluetooth activated");
                } else {
                    DisplayToastMsg("Bluetooth already ON");
                }

            }
        });
        btnStartScanning.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                bluetoothLeScanner = mBluetoothAdapter.getBluetoothLeScanner();
                arrayAdapter = new ArrayAdapter<String>(MainActivity.this, android.R.layout.select_dialog_singlechoice);
                ListScanDevice = new ArrayList<String>();
                scanLeDevice();
                AlertDialog.Builder builderSingle = new AlertDialog.Builder(MainActivity.this);
                builderSingle.setTitle("Device list: ");
                builderSingle.setNegativeButton("cancel", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });

                builderSingle.setAdapter(arrayAdapter, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Intent intent = new Intent(MainActivity.this, GattActivity.class);
                        intent.putExtra("deviceAddress", ListScanDevice.get(which));
                        startActivity(intent);
                    }
                });
                builderSingle.show();



            }
        });
    }

    public void DisplayToastMsg(String m) {
        Context context = getApplicationContext();
        CharSequence text = m;
        int duration = Toast.LENGTH_SHORT;
        Toast toast = Toast.makeText(context, text, duration);
        toast.show();
    }

    private void scanLeDevice() {

        if (!scanning) {
            // Stops scanning after a predefined scan period.
            handler.postDelayed(new Runnable() {

                @Override
                public void run() {
                    scanning = false;
                    if (ActivityCompat.checkSelfPermission(MainActivity.this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
                        ActivityCompat.requestPermissions(MainActivity.this,
                                new String[]{Manifest.permission.BLUETOOTH_SCAN}, 1);
                    }
                    bluetoothLeScanner.stopScan(leScanCallback);
                }
            }, SCAN_PERIOD);

            scanning = true;

            UUID forwardingService = UUID.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
            UUID[] forwardingServiceArray = {forwardingService};
            ScanSettings settings;
            List<ScanFilter> filters;
            settings = new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build();
            filters = new ArrayList<>();
            ParcelUuid PUuid = new ParcelUuid(forwardingService);
            ScanFilter filter = new ScanFilter.Builder().setServiceUuid(PUuid).build();
            filters.add(filter);
            if (ActivityCompat.checkSelfPermission(MainActivity.this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.BLUETOOTH_SCAN}, 1);
            }


            bluetoothLeScanner.startScan(filters, settings, leScanCallback);

            //bluetoothLeScanner.startScan(leScanCallback);
        } else {

            scanning = false;
            bluetoothLeScanner.stopScan(leScanCallback);
        }
    }


    // Device scan callback.
    private ScanCallback leScanCallback =
            new ScanCallback() {
                @Override
                public void onScanResult(int callbackType, ScanResult result) {
                    super.onScanResult(callbackType, result);

                    AddLeDeviceToArrayAdapter(arrayAdapter, result.getDevice(), result.getRssi());
                }
            };

    private void AddLeDeviceToArrayAdapter(ArrayAdapter adapter, BluetoothDevice device, int RssiValue) {
        if (ActivityCompat.checkSelfPermission(MainActivity.this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(MainActivity.this,
                    new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 1);
        }
        String deviceName = device.getName() != null ? device.getName() : "Unknown device";
        if(!(ListScanDevice.contains(device.getAddress()))){
            ListScanDevice.add(device.getAddress());
            adapter.add(deviceName + " (" + device.getAddress() + ")\n" + "RSSI = " + RssiValue +"dBm");
        }

    }

    private void ShowResultDialog(){

    }




}