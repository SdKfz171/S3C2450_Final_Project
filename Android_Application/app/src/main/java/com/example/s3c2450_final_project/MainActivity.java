package com.example.s3c2450_final_project;

import android.annotation.SuppressLint;
import android.database.DataSetObserver;
import android.os.Build;
import android.os.StrictMode;
import android.support.annotation.Nullable;
import android.support.annotation.RequiresApi;
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
import java.util.ArrayList;
import java.util.List;

import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import static android.os.StrictMode.setThreadPolicy;

public class MainActivity extends AppCompatActivity implements Runnable {
    Socket socket;
    PrintWriter output;
    BufferedReader input;

    private String ip = "192.168.103.132";
    private int port = 5555;

    Thread t = new Thread(this);

    String ACK;
    List<String> musics;

    ListView listView;
    ArrayAdapter<String> adapter;

    Button btn;
    Button servo;
    Button pause;

    boolean btn_on;
    boolean btn2_on;
    View BottomBar_Divider;
    LinearLayout BottomBar;
    TextView Music_Name;
    boolean flag=false;
    boolean playflag=false;

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

        final StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        setThreadPolicy(policy);

        t.start();

        BottomBar_Divider = (View)findViewById(R.id.divider);
        BottomBar = (LinearLayout)findViewById(R.id.Music_Bar);
        Music_Name = (TextView)findViewById(R.id.Now_Playing_Music);



        musics = new ArrayList<String>();

        listView = (ListView) findViewById(R.id.Music_List);
        adapter = new ArrayAdapter<String>(getApplicationContext(), android.R.layout.simple_list_item_1, musics);
        listView.setAdapter(adapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @RequiresApi(api = Build.VERSION_CODES.M)
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                try{

                    String selected_music = (String) parent.getItemAtPosition(position);
//
//
                    output.println("NEW");
                    output.flush();

                    output.println("PLAY" + position);
                    output.flush();

                    pause.setForeground(getDrawable(R.drawable.button_action2));
                    flag=false;


                    BottomBar_Divider.setVisibility(View.VISIBLE);
                    Music_Name.setText(selected_music);
                    BottomBar.setVisibility(View.VISIBLE);
                }
                catch (Exception e){
                    Log.d("SOCKET", "Failed Position is " + position);
                }
            }
        });


        btn = (Button) findViewById(R.id.Button01);
        servo = (Button) findViewById(R.id.Button02);
        pause = (Button) findViewById(R.id.Music_Button);


        btn_on = false;
        btn2_on = false;


        btn.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {
                try{
                    if(!btn_on){
                        output.println("C01");
                        btn.setText("LED0 OFF");
                    } else {
                        output.println("C00");
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

        servo.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {
                try{
                    if(!btn2_on){
                        output.println("SERVO");
                        servo.setText("SERVO -90");
                        btn2_on=true;
                    } else {
                        output.println("SERVO");
                        servo.setText("SERVO +90");
                        btn2_on=false;
                    }
                    output.flush();
//                    btn2_on = !btn2_on;
                    Log.d("TEST", "Socket Sended!!");
                }catch (Exception e){
                    Toast.makeText(getApplicationContext(), "문자열 전송 실패", Toast.LENGTH_SHORT).show();
                }
            }
        });
        pause.setOnClickListener(new OnClickListener() {

            @SuppressLint("NewApi")
            public void onClick(View v) {
                try{

                    output.println("PAUSE");
                    output.flush();
                    if(!flag) {
                        pause.setForeground(getDrawable(R.drawable.button_action));
                        flag = true;
                    }
                    else
                    {
                        pause.setForeground(getDrawable(R.drawable.button_action2));
                        flag=false;
                    }

                    Log.d("TEST", "Socket Sended!!");
                }catch (Exception e){
                    Toast.makeText(getApplicationContext(), "문자열 전송 실패", Toast.LENGTH_SHORT).show();
                }
            }
        });
//

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

        try {
            output.println("START");
            output.flush();
        } catch (Exception e){
            Log.d("SOCKET", "START FAILED!!");
        }

        while(true){
            try {
                ACK = input.readLine();
                Log.d("SOCKET", ACK);
//                Toast.makeText(getApplicationContext(), ACK, Toast.LENGTH_SHORT).show();
                switch (ACK) {
                    case "C00":
                        btn.setText("LED0 ON");
                        btn_on = false;
                        break;
                    case "C01":
                        btn.setText("LED0 OFF");
                        btn_on = true;
                        break;
                    case "C10":
                        servo.setText("LED1 ON");
                        btn2_on = false;
                        break;
                    case "C11":
                        servo.setText("LED1 OFF");
                        btn2_on = true;
                        break;
                    case "START":
                        output.println("LIST");
                        output.flush();
                        break;
                    case "LIST":
                        musics.clear();
                        break;
                }

                if(ACK.indexOf(".wav") > 0){
                    Log.d("SOCKET", "index of .wav : " + ACK.indexOf(".wav"));
                    Log.d("SOCKET", "length of music name : " + ACK.length());
                    Log.d("SOCKET", ACK.substring(0, ACK.indexOf(".wav")));
                    listView.setVisibility(View.VISIBLE);
                    musics.add(ACK.substring(0, ACK.indexOf(".wav")));
                    adapter.notifyDataSetChanged();
                    listView.setVisibility(View.VISIBLE);
//                    listView.invalidate();
                    Log.d("SOCKET", "music add to list");
                }

            } catch (IOException e) {
                e.printStackTrace();
            } catch (Exception e){
                e.printStackTrace();
            }
        }
    }
}
