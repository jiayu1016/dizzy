package com.wayne.dizzy.sample.sviewer;

import java.io.IOException;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ListView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.content.res.AssetManager;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.TextView;

public class MainActivity extends Activity {
    static {
        // don't need these if we use gnustl_static
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("assimp");
        // libsviewer.so will be load by framework, load it here for consistency
        System.loadLibrary("sviewer");
    }

    private OnItemClickListener mMessageClickedHandler = new OnItemClickListener() {
        public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
            String value = ((TextView)(v.findViewById(android.R.id.text1)))
                .getText().toString();
            Intent intent = new Intent(getBaseContext(),
                android.app.NativeActivity.class);
            intent.putExtra("modelName", value);
            startActivity(intent);
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        ListView listView = (ListView)findViewById(R.id.listView);

        AssetManager assetManager = getAssets();
        try {
            String[] assetsInTopDir = assetManager.list("");
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                    android.R.layout.simple_list_item_1, assetsInTopDir);
            listView.setAdapter(adapter);
            listView.setOnItemClickListener(mMessageClickedHandler);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
