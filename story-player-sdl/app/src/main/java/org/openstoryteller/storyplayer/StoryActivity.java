 
package org.openstoryteller.storyplayer;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.IOException;
import java.util.ArrayDeque;
import java.util.EnumSet;

import android.os.Bundle;
import android.util.Log;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
/* 
 * A sample wrapper class that just calls SDLActivity 
 */ 

public class StoryActivity extends SDLActivity
{
    private final static String TAG = "StoryActivity";

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    private void disconnect() {

    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onStop() {
        super.onStop();
    }


    private native void printFromJava(String txt);

    void status(String str) {
        printFromJava(str);
    }

  
 /**
     * This method is called by SDL before loading the native shared libraries.
     * It can be overridden to provide names of shared libraries to be loaded.
     * The default implementation returns the defaults. It never returns null.
     * An array returned by a new implementation must at least contain "SDL2".
     * Also keep in mind that the order the libraries are loaded may matter.
     * @return names of shared libraries to be loaded (e.g. "SDL2", "main").
     */
     
     // Actually, it *is* overridden because we generate .so files manually
      @Override
    protected String[] getLibraries() { 
        return new String[] {
            "SDL3",
            "c++_shared",
            "SDL3_image",
            // "SDL2_mixer",
            // "SDL2_net",
            // "SDL2_ttf",
            "main"
        };
    }

    @Override
    protected String getMainFunction() {
        return "main";
    }
}

