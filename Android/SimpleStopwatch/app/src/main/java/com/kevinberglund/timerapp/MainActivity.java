package com.kevinberglund.timerapp;

import android.os.Handler;
import android.os.SystemClock;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    private TextView m_tvDisplayTime = null;
    private Button m_btStartStop = null;

    private Handler m_customHandler = new Handler();

    private long m_liStartTime = 0L;
    private long m_liTimeInMilliseconds = 0L;
    private long m_liTimeSwapBuffer = 0L;
    private long m_liUpdatedTime = 0L;

    private boolean m_bStartStop = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        this.m_tvDisplayTime = (TextView)findViewById(R.id.tvTimeDisplay);
        this.m_tvDisplayTime.setText(getString(R.string.tvStartTime));

        this.m_btStartStop = (Button)findViewById(R.id.btnStartStop);
        this.m_btStartStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (m_bStartStop == false) {
                    m_btStartStop.setText(getString(R.string.btnAtMainStop));
                    m_liStartTime = SystemClock.uptimeMillis();
                    m_customHandler.postDelayed(updateTimerThread, 0);
                    m_bStartStop = true;
                } else {
                    m_btStartStop.setText(getString(R.string.btnAtMainStart));
                    m_liTimeSwapBuffer += m_liTimeInMilliseconds;
                    m_customHandler.removeCallbacks(updateTimerThread);
                    m_bStartStop = false;
                }
            }
        });
    }

    private Runnable updateTimerThread = new Runnable() {
        @Override
        public void run() {
            m_liTimeInMilliseconds = SystemClock.uptimeMillis() - m_liStartTime;

            m_liUpdatedTime = m_liTimeSwapBuffer + m_liTimeInMilliseconds;

            int secs = (int) (m_liUpdatedTime / 1000);
            int mins = secs / 60;
            secs = secs % 60;
            int milliseconds = (int) (m_liUpdatedTime % 1000);
            m_tvDisplayTime.setText("" + mins + ":"
                            + String.format("%02d", secs) + ":"
                            + String.format("%03d", milliseconds));
            m_customHandler.postDelayed(this, 0);
        }
    };
}
