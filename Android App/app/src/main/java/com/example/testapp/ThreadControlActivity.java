package com.example.testapp;

import static com.example.testapp.GattActivity.bluetoothService;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.inputmethodservice.KeyboardView;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import java.util.Timer;
import java.util.TimerTask;

public class ThreadControlActivity extends AppCompatActivity {
    String deviceAddress;
    Button btnSendMessageToggleLED;
    static int noSentPackages = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_thread_control);
        Intent intent = getIntent();
        if(null != intent){
            deviceAddress = intent.getStringExtra("deviceAddress");
        }
        btnSendMessageToggleLED = findViewById(R.id.btnSendMessageToggleLED);
        btnSendMessageToggleLED.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                    sendTestingPackages();

            }
        });



    }
    private void sendTestingPackages(){
        Timer timer = new Timer();
        noSentPackages = 0;
        timer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                if (noSentPackages == 0) {
                    bluetoothService.writeRxCharacteristic(deviceAddress + "_" + "1");
                    Log.i("ThreadControlActivity", "start");
                } else if (noSentPackages < 101) {
                    bluetoothService.writeRxCharacteristic(deviceAddress + "_" + "2");
                    Log.i("ThreadControlActivity", "toggle " + noSentPackages);

                } else {
                    bluetoothService.writeRxCharacteristic(deviceAddress + "_" + "3");
                    Log.i("ThreadControlActivity", "end");
                    timer.cancel();
                }
                noSentPackages++;
            }
            }, 0, 200);



    }
}