package fr.cloyunhee.speechanalysis;

import org.libsdl.app.SDLActivity;
import android.os.Build;
import androidx.core.content.ContextCompat;
import android.Manifest;
import android.content.pm.PackageManager;
import android.widget.Toast;
import androidx.annotation.NonNull;

public class SpeechAnalysis extends SDLActivity {

    private static final int REQUEST_AUDIO_PERMISSION_RESULT = 999876;

    private int hasPermission = -1;

    public void requestAudioPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) !=
                    PackageManager.PERMISSION_GRANTED) {
                if (shouldShowRequestPermissionRationale(Manifest.permission.RECORD_AUDIO)) {
                    Toast.makeText(this, "App required access to audio", Toast.LENGTH_SHORT).show();
                }
                requestPermissions(
                        new String[]{Manifest.permission.RECORD_AUDIO},
                        REQUEST_AUDIO_PERMISSION_RESULT);
            }
            else {
                hasPermission = 1;
            }
        } else {
            hasPermission = 1;
        }

        while (hasPermission < 0) {
            Thread.yield();
        }

        if (hasPermission == 1) {
            finish();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == REQUEST_AUDIO_PERMISSION_RESULT) {
            if (grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(getApplicationContext(), "Application will not have audio on record", Toast.LENGTH_SHORT).show();
                hasPermission = 0;
            }
            else {
                hasPermission = 1;
            }
        }
    }
    

}
