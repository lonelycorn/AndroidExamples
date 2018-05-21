package com.lonelycorn.inputhandler;

import android.app.Activity;
import android.os.Bundle;
import android.view.KeyEvent;
import android.widget.TextView;

public class MainActivity extends Activity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private TextView keyEventString;
    private TextView keyEventCharacters;
    private TextView keyEventKeyCode;
    private TextView keyEventUnicodeChar;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        keyEventString = findViewById(R.id.key_event_string);
        keyEventCharacters = findViewById(R.id.key_event_characters);
        keyEventKeyCode = findViewById(R.id.key_event_key_code);
        keyEventUnicodeChar = findViewById(R.id.key_event_unicode_char);
    }

    // return true if the key was consumed
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {

        keyEventString.setText(event.toString());
        if ((event.getAction() == KeyEvent.ACTION_MULTIPLE) &&
                (event.getKeyCode() == KeyEvent.KEYCODE_UNKNOWN)) {
            keyEventCharacters.setText(String.format("'%s'", event.getCharacters()));
        } else {
            keyEventCharacters.setText("N/A");
        }
        keyEventKeyCode.setText(String.format("%d", event.getKeyCode()));
        keyEventUnicodeChar.setText((String.format("%d", event.getUnicodeChar())));

        return super.dispatchKeyEvent(event);
    }
}
