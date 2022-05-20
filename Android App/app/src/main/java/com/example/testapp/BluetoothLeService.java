package com.example.testapp;

import static android.nfc.NfcAdapter.EXTRA_DATA;

import android.Manifest;
import android.app.Activity;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.UUID;

public class BluetoothLeService extends Service {
    public static final String TAG = "BluetoothLeService";

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothGatt bluetoothGatt;



    private static BluetoothGattCharacteristic mRxCharacteristic;
    private static BluetoothGattCharacteristic mTxCharacteristic;
    private static BluetoothGattDescriptor mTxCccd;
    private final static String CccdUUID = "00002902-0000-1000-8000-00805f9b34fb";

    public final static String ACTION_GATT_CONNECTED =
            "com.example.bluetooth.le.ACTION_GATT_CONNECTED";
    public final static String ACTION_GATT_DISCONNECTED =
            "com.example.bluetooth.le.ACTION_GATT_DISCONNECTED";
    public final static String ACTION_GATT_SERVICES_DISCOVERED =
            "com.example.bluetooth.le.ACTION_GATT_SERVICES_DISCOVERED";
    public final static String ACTION_DATA_AVAILABLE =
            "com.example.bluetooth.le.ACTION_DATA_AVAILABLE";
    public final static String EXTRA_DATA =
            "com.example.bluetooth.le.EXTRA_DATA";
    public final static String EXTRA_ADDRESS =
            "com.example.bluetooth.le.EXTRA_ADDRESS";
    public final static String EXTRA_NUMBER_OF_DEVICES =
            "com.example.bluetooth.le.EXTRA_NUMBER_OF_DEVICES";
    public final static String EXTRA_PACKETS =
            "com.example.bluetooth.le.EXTRA_PACKETS";

    public final static UUID UUID_HEART_RATE_MEASUREMENT =
            UUID.fromString(SampleGattAttributes.HEART_RATE_MEASUREMENT);

    private static final int STATE_DISCONNECTED = 0;
    private static final int STATE_CONNECTED = 2;

    public Queue<BluetoothGattCharacteristic> characteristicQueue;
    private int connectionState;

    public boolean initialize() {
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
            Log.e(TAG, "Unable to obtain a BluetoothAdapter.");
            return false;
        }
        return true;
    }

    public boolean connect(final String address) {
        if (bluetoothAdapter == null || address == null) {
            Log.w(TAG, "BluetoothAdapter not initialized or unspecified address.");
            return false;
        }

        try {
            final BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);

            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions((Activity) getApplicationContext(),
                        new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 1);

            }
            bluetoothGatt = device.connectGatt(this, false, bluetoothGattCallback);
            return true;
        } catch (IllegalArgumentException exception) {
            Log.w(TAG, "Device not found with provided address.");
            return false;
        }
        // connect to the GATT server on the device
    }

    private final BluetoothGattCallback bluetoothGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                // successfully connected to the GATT Server
                connectionState = STATE_CONNECTED;
                broadcastUpdate(ACTION_GATT_CONNECTED);
                if (ActivityCompat.checkSelfPermission(getApplicationContext(), Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                    ActivityCompat.requestPermissions((Activity) getApplicationContext(),
                            new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 1);

                }
                bluetoothGatt.discoverServices();
                Log.i("Thu", "discoverServices()");
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                // disconnected from the GATT Server
                connectionState = STATE_DISCONNECTED;
                broadcastUpdate(ACTION_GATT_DISCONNECTED);
                Log.i("Thu", "broadcastUpdate()");
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                characteristicQueue = new LinkedList<BluetoothGattCharacteristic>();
                Log.i("onServicesDiscovered", "status == BluetoothGatt.GATT_SUCCESS");


                broadcastUpdate(ACTION_GATT_SERVICES_DISCOVERED);
            } else {
                Log.w(TAG, "onServicesDiscovered received: " + status);
            }
        }

        @Override
        public void onCharacteristicRead(
                BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic,
                int status
        ) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
            }
            if (!characteristicQueue.isEmpty()) {
                if (ActivityCompat.checkSelfPermission(getApplicationContext(), Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                    ActivityCompat.requestPermissions((Activity) getApplicationContext(),
                            new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 1);
                }
                bluetoothGatt.readCharacteristic(characteristicQueue.peek());
                characteristicQueue.remove();
            }
        }

        @Override
        public void onCharacteristicChanged(
                BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic
        ) {
            String uuid = characteristic.getUuid().toString();
            String s;

            if(uuid.equalsIgnoreCase(SampleGattAttributes.CHARACTERISTIC_TX_UUID)) {
                s = characteristic.getStringValue(0);
                Log.i("Thu", "string s " + s);
                broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
            }


        }
    };

    public List<BluetoothGattService> getSupportedGattServices() {
        if (bluetoothGatt == null) return null;
        return bluetoothGatt.getServices();
    }

    private void broadcastUpdate(final String action) {
        final Intent intent = new Intent(action);
        sendBroadcast(intent);
    }

    private void broadcastUpdate(final String action,
                                 final BluetoothGattCharacteristic characteristic) {
        final Intent intent = new Intent(action);

        // This is special handling for the Heart Rate Measurement profile. Data
        // parsing is carried out as per profile specifications.
        if (UUID_HEART_RATE_MEASUREMENT.equals(characteristic.getUuid())) {
            int flag = characteristic.getProperties();
            int format = -1;
            if ((flag & 0x01) != 0) {
                format = BluetoothGattCharacteristic.FORMAT_UINT16;
                Log.d(TAG, "Heart rate format UINT16.");
            } else {
                format = BluetoothGattCharacteristic.FORMAT_UINT8;
                Log.d(TAG, "Heart rate format UINT8.");
            }
            final int heartRate = characteristic.getIntValue(format, 1);
            Log.d(TAG, String.format("Received heart rate: %d", heartRate));
            intent.putExtra(EXTRA_DATA, String.valueOf(heartRate));
        } else {
            // For all other profiles, writes the data formatted in HEX.
            final byte[] data = characteristic.getValue();
            if (data != null && data.length > 5) {
                final StringBuilder stringBuilder = new StringBuilder(data.length);
                for (int byteIndex = 0; byteIndex < data.length; byteIndex++){
                    stringBuilder.append(String.format("%02X", data[byteIndex]));
                    if(byteIndex % 2 != 0 && byteIndex < data.length - 1){
                        stringBuilder.append(":");
                    }

                }
                //intent.putExtra(EXTRA_DATA, new String(data) + "\n" +
                //        stringBuilder.toString());

                if(data.length == 16){
                    //threadDeviceAddressList.add(stringBuilder.toString());
                    intent.putExtra(EXTRA_ADDRESS, stringBuilder.toString());
                }
                else{
                    //numberOfThreadDevice = Integer.parseInt(String.format("%02X", data[0]),16);
                    intent.putExtra(EXTRA_NUMBER_OF_DEVICES, Integer.parseInt(String.format("%02X", data[0]),16));
                }
            }
            else if(data != null && data.length == 1){
                intent.putExtra(EXTRA_PACKETS, data.toString());
            }
        }
        sendBroadcast(intent);
    }
    /*
    public void readCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (bluetoothGatt == null) {
            Log.w(TAG, "BluetoothGatt not initialized");
            return;
        }
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions((Activity) getApplicationContext(),
                    new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 1);
        }
        bluetoothGatt.readCharacteristic(characteristic);
    }

    */

    public void readCharacteristic(ArrayList<ArrayList<BluetoothGattCharacteristic>> mGattCharacteristics) {
        if (bluetoothGatt == null) {
            Log.w(TAG, "BluetoothGatt not initialized");
            return;
        }
        for (ArrayList<BluetoothGattCharacteristic> arrSubList : mGattCharacteristics) {
            for (BluetoothGattCharacteristic characteristic : arrSubList) {
                characteristicQueue.offer(characteristic);
            }
        }
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions((Activity) getApplicationContext(),
                    new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 1);
        }
        bluetoothGatt.readCharacteristic(characteristicQueue.peek());
        characteristicQueue.remove();
    }

    public void setCharacteristicNotification(BluetoothGattCharacteristic characteristic, boolean enabled) {
        if (bluetoothGatt == null) {
            Log.w(TAG, "BluetoothGatt not initialized");
            return;
        }
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions((Activity) getApplicationContext(),
                    new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 1);
        }
        bluetoothGatt.setCharacteristicNotification(characteristic, enabled);

        // This is specific to Heart Rate Measurement.
        if (UUID_HEART_RATE_MEASUREMENT.equals(characteristic.getUuid())) {
            BluetoothGattDescriptor descriptor = characteristic.getDescriptor(UUID.fromString(SampleGattAttributes.CLIENT_CHARACTERISTIC_CONFIG));
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            bluetoothGatt.writeDescriptor(descriptor);
        }
    }

    public void writeRxCharacteristic(String command) {
        BluetoothGattService mService = bluetoothGatt.getService(UUID.fromString(SampleGattAttributes.SERVICE_FORWARDING_UUID));
        mRxCharacteristic = mService.getCharacteristic(UUID.fromString(SampleGattAttributes.CHARACTERISTIC_RX_UUID));
        Log.i(TAG, command);
        mRxCharacteristic.setValue(command);
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions((Activity) getApplicationContext(),
                    new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 1);
        }
        bluetoothGatt.writeCharacteristic(mRxCharacteristic);
    }
    public void writeNotification(boolean value) {
        BluetoothGattService mService = bluetoothGatt.getService(UUID.fromString(SampleGattAttributes.SERVICE_FORWARDING_UUID));
        mTxCharacteristic = mService.getCharacteristic(UUID.fromString(SampleGattAttributes.CHARACTERISTIC_TX_UUID));
        mTxCccd = mTxCharacteristic.getDescriptor(UUID.fromString(CccdUUID));
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions((Activity) getApplicationContext(),
                    new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 1);
        }
        bluetoothGatt.setCharacteristicNotification(mTxCharacteristic, value);


        byte[] byteVal = new byte[1];
        if (value) {
            //byteVal[0] = 1;
            mTxCccd.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        } else {
            //byteVal[0] = 0;
            mTxCccd.setValue(BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
        }

        //bluetoothGatt.writeDescriptor(mTxCccd);
        Log.d(TAG, "writeDescriptor: " + String.valueOf(bluetoothGatt.writeDescriptor(mTxCccd)));


    }

    private Binder binder = new LocalBinder();

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    class LocalBinder extends Binder {
        public BluetoothLeService getService() {
            return BluetoothLeService.this;
        }
    }

    @Override
    public boolean onUnbind(Intent intent) {
        close();
        return super.onUnbind(intent);
    }

    private void close() {
        if (bluetoothGatt == null) {
            return;
        }
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            return;
        }
        bluetoothGatt.close();
        bluetoothGatt = null;
    }

}