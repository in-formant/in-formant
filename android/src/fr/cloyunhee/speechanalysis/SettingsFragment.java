package fr.cloyunhee.speechanalysis;

import android.os.*;
import android.content.*;
import android.util.*;
import android.view.*;
import android.widget.*;
import androidx.preference.*;
import java.util.*;

public class SettingsFragment extends PreferenceFragmentCompat {

    public static final int MIN_FFT = 64;
    public static final int MAX_FFT = 2048;

    public static final int MIN_LP = 5;
    public static final int MAX_LP = 14;

    public static final int MIN_FREQ = 2500;
    public static final int MAX_FREQ = 7000;

    public static final int MIN_FRM_LENGTH = 25;
    public static final int MAX_FRM_LENGTH = 60;

    public static final int MIN_FRM_SPACE = 10;
    public static final int MAX_FRM_SPACE = 25;

    public static final int MIN_WIN_SPAN = 2;
    public static final int MAX_WIN_SPAN = 10;

    private static final SuffixSummaryProvider hertzSuffix = new SuffixSummaryProvider("Hz");
    private static final SuffixSummaryProvider millisecondSuffix = new SuffixSummaryProvider("ms");
    private static final SuffixSummaryProvider secondSuffix = new SuffixSummaryProvider("s");

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootPreferenceKey) {
        final Context activityContext = getActivity();

        final PreferenceScreen preferenceScreen = getPreferenceManager().createPreferenceScreen(activityContext);

        final TypedValue themeTypedValue = new TypedValue();
        activityContext.getTheme().resolveAttribute(R.attr.preferenceTheme, themeTypedValue, true);
        final ContextThemeWrapper contextThemeWrapper = new ContextThemeWrapper(activityContext, themeTypedValue.resourceId);

        setPreferenceScreen(preferenceScreen);
        createAnalysisCategory(contextThemeWrapper);
        createDisplayCategory(contextThemeWrapper);
    }

    private void createAnalysisCategory(final ContextThemeWrapper context) {
        final PreferenceCategory analysis = new PreferenceCategory(context);
        {
            analysis.setTitle("Analysis settings");
        }
        getPreferenceScreen().addPreference(analysis);

        final DropDownPreference fftSize = new DropDownPreference(context);
        {
            fftSize.setPersistent(false);
            fftSize.setKey("fftsize");
            fftSize.setTitle("FFT size");
            fftSize.setSummaryProvider(ListPreference.SimpleSummaryProvider.getInstance());

            fftSize.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                public boolean onPreferenceChange(Preference preference, Object newValue) {
                    JniBridge.setFftSize(Integer.parseInt(newValue.toString()));
                    return true;
                }
            });
            
            final List<String> nfftList = new ArrayList<>();
            for (int nfft = MIN_FFT; nfft <= MAX_FFT; nfft *= 2) {
                nfftList.add(Integer.toString(nfft));
            }
            String[] nfftArray = nfftList.toArray(new String[0]);

            fftSize.setEntries(nfftArray);
            fftSize.setEntryValues(nfftArray);
            fftSize.setValue(Integer.toString(JniBridge.getFftSize()));
        }
        analysis.addPreference(fftSize);
        
        final DropDownPreference lpOrder = new DropDownPreference(context);
        {
            lpOrder.setPersistent(false);
            lpOrder.setKey("lporder");
            lpOrder.setTitle("Linear prediction order");
            lpOrder.setSummaryProvider(ListPreference.SimpleSummaryProvider.getInstance());
    
            lpOrder.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                public boolean onPreferenceChange(Preference preference, Object newValue) {
                    JniBridge.setLpOrder(Integer.parseInt(newValue.toString()));
                    return true;
                }
            });
 
            final List<String> entryList = new ArrayList<>();
            for (int n = MIN_LP; n <= MAX_LP; n++) {
                entryList.add(Integer.toString(n));
            }
            String[] entryArray = entryList.toArray(new String[0]);

            lpOrder.setEntries(entryArray);
            lpOrder.setEntryValues(entryArray);
            lpOrder.setValue(Integer.toString(JniBridge.getLpOrder()));
        }
        analysis.addPreference(lpOrder);
       
        final SeekBarPreference maxFreq = new SeekBarPreference(context);
        {
            maxFreq.setPersistent(false);
            maxFreq.setKey("maxfreq");
            maxFreq.setTitle("Maximum formant frequency");
            maxFreq.setUpdatesContinuously(true);

            maxFreq.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                public boolean onPreferenceChange(Preference preference, Object newValue) {
                    JniBridge.setMaxFreq(Integer.parseInt(newValue.toString()));
                    maxFreq.setSummary(String.format("%d Hz", newValue));
                    return true;
                }
            });

            maxFreq.setMin(MIN_FREQ);
            maxFreq.setMax(MAX_FREQ);
            maxFreq.setSeekBarIncrement(50);
            int value = JniBridge.getMaxFreq();
            maxFreq.setValue(value);
            maxFreq.setSummary(String.format("%d Hz", value));
        }
        analysis.addPreference(maxFreq);

        final SeekBarPreference frameLength = new SeekBarPreference(context);
        {
            frameLength.setPersistent(false);
            frameLength.setKey("framelength");
            frameLength.setTitle("Frame length");
            frameLength.setUpdatesContinuously(true);

            frameLength.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                public boolean onPreferenceChange(Preference preference, Object newValue) {
                    JniBridge.setFrameLength(Integer.parseInt(newValue.toString()));
                    frameLength.setSummary(String.format("%d ms", newValue));
                    return true;
                }
            });

            frameLength.setMin(MIN_FRM_LENGTH);
            frameLength.setMax(MAX_FRM_LENGTH);
            frameLength.setSeekBarIncrement(1);
            int value = JniBridge.getFrameLength();
            frameLength.setValue(value);
            frameLength.setSummary(String.format("%d ms", value));
        }
        analysis.addPreference(frameLength);

        final SeekBarPreference frameSpace = new SeekBarPreference(context);
        {
            frameSpace.setPersistent(false);
            frameSpace.setKey("framespace");
            frameSpace.setTitle("Frame space");
            frameSpace.setUpdatesContinuously(true);

            frameSpace.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                public boolean onPreferenceChange(Preference preference, Object newValue) {
                    JniBridge.setFrameSpace(Integer.parseInt(newValue.toString()));
                    frameSpace.setSummary(String.format("%d ms", newValue));
                    return true;
                }
            });

            frameSpace.setMin(MIN_FRM_SPACE);
            frameSpace.setMax(MAX_FRM_SPACE);
            frameSpace.setSeekBarIncrement(1);
            int value = JniBridge.getFrameSpace();
            frameSpace.setValue(value);
            frameSpace.setSummary(String.format("%d ms", value));
        }
        analysis.addPreference(frameSpace);

        final SeekBarPreference duration = new SeekBarPreference(context);
        {
            duration.setPersistent(false);
            duration.setKey("duration");
            duration.setTitle("Duration");
            duration.setUpdatesContinuously(true);

            duration.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                public boolean onPreferenceChange(Preference preference, Object newValue) {
                    JniBridge.setDuration(Integer.parseInt(newValue.toString()));
                    duration.setSummary(String.format("%d s", newValue));
                    return true;
                }
            });

            duration.setMin(MIN_WIN_SPAN);
            duration.setMax(MAX_WIN_SPAN);
            duration.setSeekBarIncrement(1);
            int value = JniBridge.getDuration();
            duration.setValue(value);
            duration.setSummary(String.format("%d s", value));
        }
        analysis.addPreference(duration);

    }

    private void createDisplayCategory(final Context context) {
        final PreferenceCategory display = new PreferenceCategory(context);
        {
            display.setTitle("Display settings");
        }
        getPreferenceScreen().addPreference(display);

        final RangeSeekBarPreference gain = new RangeSeekBarPreference(context);
        {
            gain.setPersistent(false);
            gain.setKey("gain");
            gain.setTitle("Display gain");
            gain.setUpdatesContinuously(true);

            gain.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                public boolean onPreferenceChange(Preference preference, Object newValue) {
                    FloatPair range = (FloatPair) newValue;
                    int lo = (int) range.first;
                    int hi = (int) range.second;

                    JniBridge.setMinGain(lo);
                    JniBridge.setMaxGain(hi);

                    gain.setSummary(String.format("Minimum: %d dB\nMaximum: %d dB", lo, hi));
                    return true;
                }
            });

            gain.setMin(-80);
            gain.setMax(60);
            int lo = JniBridge.getMinGain();
            int hi = JniBridge.getMaxGain();
            gain.setProgress(lo, hi);
            gain.setSummary(String.format("Minimum: %d dB\nMaximum: %d dB", lo, hi));
        }
        display.addPreference(gain);

        final ColorPickerPreference pitchColor = new ColorPickerPreference(context);
        {
            pitchColor.setPersistent(false);
            pitchColor.setKey("pitchcolor");
            pitchColor.setTitle("Pitch color");

            pitchColor.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                public boolean onPreferenceChange(Preference preference, Object newValue) {
                    JniBridge.setPitchColor(Integer.parseInt(newValue.toString()));
                    return true;
                }
            });
                        
            pitchColor.setDefaultValue(JniBridge.getPitchColor());
        }
        display.addPreference(pitchColor);
        
        final int formantCount = JniBridge.getFormantCount();

        for (int formantNb = 0; formantNb < formantCount; ++formantNb) {

            // Needs to be final for the change listener.
            final int nb = formantNb;

            final ColorPickerPreference formantColor = new ColorPickerPreference(context);
            {
                formantColor.setPersistent(false);
                formantColor.setKey(String.format("f%dcolor", nb + 1));
                formantColor.setTitle(String.format("F%d color", nb + 1));

                formantColor.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                    public boolean onPreferenceChange(Preference preference, Object newValue) {
                        JniBridge.setFormantColor(nb, Integer.parseInt(newValue.toString()));
                        return true;
                    }
                });
                            
                formantColor.setDefaultValue(JniBridge.getFormantColor(nb));
            }
            display.addPreference(formantColor);

        }


    }

    private static class SuffixSummaryProvider implements Preference.SummaryProvider<SeekBarPreference> {
        private final String mSuffix;

        public SuffixSummaryProvider(String suffix) {
            mSuffix = suffix;
        }

        public CharSequence provideSummary(SeekBarPreference preference) {
            return String.format("%d %s", preference.getValue(), mSuffix);
        }
    }

}

