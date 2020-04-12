package fr.cloyunhee.speechanalysis;

import android.content.Context;
import android.content.res.TypedArray;
import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.View;
import android.widget.TextView;
import androidx.preference.Preference;
import androidx.preference.PreferenceViewHolder;
import com.jaygoo.widget.RangeSeekBar;
import com.jaygoo.widget.SeekBar;
import com.jaygoo.widget.OnRangeChangedListener;

class RangeSeekBarPreference extends Preference implements OnRangeChangedListener, View.OnKeyListener {

    private RangeSeekBar mSeekBar;

    private float mMin, mMax;
    private float mLo, mHi;
    private boolean mUpdatesContinuously;
    private boolean mTrackingTouch;

    public RangeSeekBarPreference(Context context) {
        super(context);

        mSeekBar = null;
        mMin = 0;
        mMax = 5;
        mLo = 1;
        mHi = 3;

        setLayoutResource(R.layout.seekbar);
    }

    @Override
    public void onBindViewHolder(PreferenceViewHolder view) {
        super.onBindViewHolder(view);
        view.itemView.setOnKeyListener(this);
        mSeekBar = (RangeSeekBar) view.findViewById(R.id.seekbar);
        mSeekBar.setOnRangeChangedListener(this);
        mSeekBar.setSeekBarMode(RangeSeekBar.SEEKBAR_MODE_RANGE);
        mSeekBar.setRange(mMin, mMax, 10);
        mSeekBar.setProgress(mLo, mHi); 
    }

    @Override
    protected void onSetInitialValue(boolean restoreValue, Object defaultValue) {
        FloatPair range = (FloatPair) defaultValue;
        setProgress(range.first, range.second);
    }

    @Override
    protected Object onGetDefaultValue(TypedArray a, int index) {
        return new FloatPair(mLo, mHi);
    }

    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event) {
        if (event.getAction() != KeyEvent.ACTION_DOWN) {
            return false;
        }

        if (mSeekBar == null) {
            return false;
        }
        return mSeekBar.onKeyDown(keyCode, event);
    }

    public void setMax(float max) {
        if (max != mMax) {
            mMax = max;
            notifyChanged();
        }
    }

    public void setMin(float min) {
        if (min != mMin) {
            mMin = min;
            notifyChanged();
        }
    }

    public float getMax() {
        return mMax;
    }

    public float getMin() {
        return mMin;
    }

    public void setProgress(float lo, float hi) {
        setProgress(lo, hi, true);
    }

    public void setUpdatesContinuously(boolean update) {
        mUpdatesContinuously = update;
    }

    private void setProgress(float lo, float hi, boolean notifyChanged) {
        if (lo < mMin) {
            lo = mMin;
        }
        if (hi > mMax) {
            hi = mMax;
        }

        if (lo > hi) {
            lo = hi;
        }
        if (lo == hi) {
            lo -= 10;
        }

        if (lo != mLo || hi != mHi) {
            mLo = lo;
            mHi = hi;
            if (notifyChanged) {
                notifyChanged();
            }
        }
    }

    public float getLo() {
        return mLo;
    }

    public float getHi() {
        return mHi;
    }
    
    private void syncProgress(RangeSeekBar seekBar) {
        float lo = seekBar.getLeftSeekBar().getProgress();
        float hi = seekBar.getRightSeekBar().getProgress();

        if (lo != mLo || hi != mHi) {
            if (callChangeListener(new FloatPair(lo, hi))) {
                setProgress(lo, hi, false);
            }
            else {
                seekBar.setProgress(lo);
            }
        }
    }

    @Override
    public void onRangeChanged(RangeSeekBar seekBar, float lo, float hi, boolean fromUser) {
        if (fromUser && (mUpdatesContinuously || !mTrackingTouch)) {
            syncProgress(seekBar);
        }
    }

    @Override
    public void onStartTrackingTouch(RangeSeekBar seekBar, boolean isLeft) {
        mTrackingTouch = true;
    }

    @Override
    public void onStopTrackingTouch(RangeSeekBar seekBar, boolean isLeft) {
        mTrackingTouch = false;
        syncProgress(seekBar);
    }

}
