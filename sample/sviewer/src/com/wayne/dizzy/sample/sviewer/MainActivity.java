package com.wayne.dizzy.sample.sviewer;

import com.wayne.dizzy.sample.sviewer.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class MainActivity extends Activity {
    static {
        // don't need these if we use gnustl_static
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("aol");
        System.loadLibrary("are");
        System.loadLibrary("sviewer");
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        Button startBtn = (Button) findViewById(R.id.btn_id_start_native_activity);
        startBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(getBaseContext(),
                        android.app.NativeActivity.class));
            }
        });
    }
}
