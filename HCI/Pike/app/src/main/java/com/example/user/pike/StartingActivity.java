package com.example.user.pike;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.Menu;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ImageButton;
import android.widget.Spinner;

/**
 * Created by user on 2017/1/2.
 */

public class StartingActivity extends AppCompatActivity {

    private ImageButton imageButton2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_starting);
        //getSupportActionBar().hide();


        Spinner dropdown = (Spinner)findViewById(R.id.spinner1);
        String[] items = new String[]{"National Taiwan University", "National Chiao Tung University", "National Tsing Hua University", "National Chengchi University"};
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, items);
        dropdown.setAdapter(adapter);

        imageButton2 = (ImageButton) findViewById(R.id.imageButton2);
        imageButton2.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent  = new Intent();
                intent.setClass(StartingActivity.this, MapsActivity.class);
                startActivity(intent);
            }
        });
    }

}
