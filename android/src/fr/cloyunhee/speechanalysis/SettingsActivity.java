package fr.cloyunhee.speechanalysis;

import android.os.*;
import androidx.fragment.app.*;

public class SettingsActivity extends FragmentActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.settings);
        getSupportFragmentManager()
                .beginTransaction()
                .replace(R.id.main_content, new SettingsFragment())
                .commit();
    }

    @Override
    protected void onResume() {
        super.onResume();
        JniBridge.toggleAnalysis(false);
    }

    @Override
    protected void onStop() {
        super.onStop();
        JniBridge.toggleAnalysis(true);
    }

}
