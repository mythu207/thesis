package com.example.testapp;

import static com.example.testapp.GattActivity.bluetoothService;

import androidx.appcompat.app.AppCompatActivity;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;


import java.util.Timer;
import java.util.TimerTask;

public class ModeSelectActivity extends AppCompatActivity {
    String TAG = "ModeSelectActitivity";
    String deviceAddress;
    Button btnTestingMode, btnControlMode;
    static int noSentPackages = 0;
    static int noReceivedPackages = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mode_select);
        setTitle("Thread Device Control");
        Intent intent = getIntent();
        if(null != intent){
            deviceAddress = intent.getStringExtra("deviceAddress");
            btnTestingMode = findViewById(R.id.btnTestingMode);
            btnControlMode = findViewById(R.id.btnControlMode);

            btnTestingMode.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    sendTestingPackages();
                }
            });

            btnControlMode.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    sendControlPackage();


                }
            });
        }
    }
    private void sendTestingPackages(){
        Timer timer = new Timer();
        noSentPackages = 1;
        noReceivedPackages = 0;
        timer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                if (noSentPackages < 101) {
                    bluetoothService.writeRxCharacteristic(deviceAddress + "_" + "A");
                    Log.i("ModeSelectActivity", "start");
                }
                else {
                    Log.i("ModeSelectActivity", "end");
                    timer.cancel();

                }
                noSentPackages++;
            }
        }, 0, 200);

    }
    private void sendControlPackage() {
        bluetoothService.writeRxCharacteristic(deviceAddress + "_" + "B");
    }


}