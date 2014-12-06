package com.wayne.dizzy.sample.sviewer;

import java.io.File;
import java.io.IOException;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.content.res.AssetManager;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.TextView;

public class MainActivity extends Activity {
    private static final String TAG = "SceneViewer";

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
            if (value.startsWith("/", 0))
                intent.putExtra("isAsset", false);
            else
                intent.putExtra("isAsset", true);
            startActivity(intent);
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        ListView listView = (ListView)findViewById(R.id.listView);

        try {
            AssetManager assetManager = getAssets();
            String[] assetsInTopDir = assetManager.list("");
            String path = Environment.getExternalStorageDirectory()
                .toString() +"/SceneViwer";

            File dir = new File(path);
            if (!dir.exists()) {
                Log.d(TAG, "Create dir: " + path);
                dir.mkdir();
            }
            File file[] = dir.listFiles();
            String[] allFiles = new String[assetsInTopDir.length + file.length];
            int count = 0;
            for (int i=0; i < assetsInTopDir.length; i++) {
                allFiles[count++] = assetsInTopDir[i];
            }
            for (int i=0; i < file.length; i++) {
                allFiles[count++] = file[i].getAbsolutePath();
            }

            ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                    android.R.layout.simple_list_item_1, allFiles);
            listView.setAdapter(adapter);
            listView.setOnItemClickListener(mMessageClickedHandler);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
