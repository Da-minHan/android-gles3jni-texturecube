/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.gles3jni;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

import java.io.File;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;

public class GLES3JNIActivity extends Activity {

    private static String LOGTAG = "GLES3JNIActivity";
    GLES3JNIView mView;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        String privateAssetDirectory = getFilesDir().getAbsolutePath();
        extractAsset("container.bmp", privateAssetDirectory);
        extractAsset("cube_map.bmp", privateAssetDirectory);
        extractAsset("back.jpg", privateAssetDirectory);

        Log.d(LOGTAG, privateAssetDirectory + " privateAssetDirectory \n");

        mView = new GLES3JNIView(getApplication());
        setContentView(mView);
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    private void extractAsset(String assetName, String assetPath)
    {
        File fileTest = new File(assetPath, assetName);

        if(fileTest.exists())
        {
            Log.d(LOGTAG, assetName +  " already exists no extraction needed\n");
        }
        else
        {
            Log.d(LOGTAG, assetName + " doesn't exist extraction needed \n");
            try
            {
                RandomAccessFile out = new RandomAccessFile(fileTest,"rw");
                AssetManager am = getResources().getAssets();
                InputStream inputStream = am.open(assetName);

                Bitmap bitmap = BitmapFactory.decodeStream(inputStream);
                int bytes = bitmap.getByteCount();
//or we can calculate bytes this way. Use a different value than 4 if you don't use 32bit images.
//int bytes = b.getWidth()*b.getHeight()*4;

                ByteBuffer buffer = ByteBuffer.allocate(bytes); //Create a new buffer
                bitmap.copyPixelsToBuffer(buffer); //Move the byte data to the buffer

                byte[] array = buffer.array();
                out.write(array);

                out.close();
                inputStream.close();
            }
            catch(Exception e)
            {
                Log.e(LOGTAG, "Failure in extractAssets(): " + e.toString() + " " + assetPath+assetName);
            }
            if(fileTest.exists())
            {
                Log.d(LOGTAG,"File Extracted successfully");
            }
        }
    }
}
