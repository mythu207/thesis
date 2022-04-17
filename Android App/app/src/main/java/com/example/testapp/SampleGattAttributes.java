package com.example.testapp;

import java.util.HashMap;

public class SampleGattAttributes {
    private static HashMap<String, String> attributes = new HashMap();
    public static String HEART_RATE_MEASUREMENT = "00002a37-0000-1000-8000-00805f9b34fb";
    public static String CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";

    public static String SERVICE_FORWARDING_NAME = "BLE/Thread forwarding service";
    public static String SERVICE_FORWARDING_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";

    public static String CHARACTERISTIC_RX_NAME = "Rx characteristic";
    public static String CHARACTERISTIC_RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
    public static String CHARACTERISTIC_TX_NAME = "Tx characteristic";
    public static String CHARACTERISTIC_TX_UUID = "6e400004-b5a3-f393-e0a9-e50e24dcca9e";

    static {
        // Sample Services.
        attributes.put(SERVICE_FORWARDING_UUID, SERVICE_FORWARDING_NAME);
        attributes.put("0000180a-0000-1000-8000-00805f9b34fb", "Device Information Service");
        // Sample Characteristics.
        attributes.put("00002a29-0000-1000-8000-00805f9b34fb", "Manufacturer Name String");
        attributes.put(CHARACTERISTIC_RX_UUID, CHARACTERISTIC_RX_NAME);
        attributes.put(CHARACTERISTIC_TX_UUID, CHARACTERISTIC_TX_NAME);

    }

    public static String lookup(String uuid, String defaultName) {
        String name = attributes.get(uuid);
        return name == null ? defaultName : name;
    }

    public static String lookup(String uuid) {
        String name = attributes.get(uuid);
        return name == null ? "Unknown" : name;
    }


}
