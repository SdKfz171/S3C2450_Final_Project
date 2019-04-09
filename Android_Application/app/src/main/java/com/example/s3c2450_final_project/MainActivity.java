package com.example.s3c2450_final_project;

import android.os.StrictMode;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.Socket;
import java.nio.Buffer;

import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

import static android.os.StrictMode.setThreadPolicy;

public class MainActivity extends AppCompatActivity implements Runnable {
    Socket socket;
    PrintWriter output;
    BufferedReader input;

    private String ip = "192.168.103.99";
    private int port = 5555;

    Thread t = new Thread(this);

    String ACK;

    Button btn;
    Button btn2;

    boolean btn_on;
    boolean btn2_on;

    @Override
    protected void onStop() {
        super.onStop();
        try {
            socket.close();
            output.close();
            input.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        try {
            socket = new Socket(ip, port);
            output = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            input = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            Toast.makeText(this, "소켓 재 연결", Toast.LENGTH_SHORT).show();
        } catch (IOException e) {
            Toast.makeText(this, "소켓 재 연결 실패", Toast.LENGTH_SHORT).show();
            e.printStackTrace();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        setThreadPolicy(policy);

        t.start();

        btn = (Button) findViewById(R.id.Button01);
        btn2 = (Button) findViewById(R.id.Button02);

        btn_on = false;
        btn2_on = false;

        btn.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {
                try{
                    if(!btn_on){
                        output.println("c01");
                        btn.setText("LED0 OFF");
                    } else {
                        output.println("c00");
                        btn.setText("LED0 ON");
                    }
                    output.flush();
//                    btn_on = !btn_on;
                    Log.d("TEST", "Socket Sended!!");
                }catch (Exception e){
                    Toast.makeText(getApplicationContext(), "문자열 전송 실패", Toast.LENGTH_SHORT).show();
                }
            }
        });

        btn2.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {
                try{
                    if(!btn2_on){
                        output.println("c11");
                        btn2.setText("LED1 OFF");
                    } else {
                        output.println("c10");
                        btn2.setText("LED1 ON");
                    }
                    output.flush();
//                    btn2_on = !btn2_on;
                    Log.d("TEST", "Socket Sended!!");
                }catch (Exception e){
                    Toast.makeText(getApplicationContext(), "문자열 전송 실패", Toast.LENGTH_SHORT).show();
                }
            }
        });


    }

    @Override
    public void run() {
        try {
            socket = new Socket(ip, port);
            output = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            input = new BufferedReader(new InputStreamReader(socket.getInputStream()));

        }catch (Exception e) {
            Toast.makeText(this, "서버 연결 실패", Toast.LENGTH_LONG).show();
        }
        while(true){
            try {
                ACK = input.readLine();
                switch (ACK) {
                    case "c00":
                        btn.setText("LED0 ON");
                        btn_on = false;
                        break;
                    case "c01":
                        btn.setText("LED0 OFF");
                        btn_on = true;
                        break;
                    case "c10":
                        btn2.setText("LED1 ON");
                        btn2_on = false;
                        break;
                    case "c11":
                        btn2.setText("LED1 OFF");
                        btn2_on = true;
                        break;
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
