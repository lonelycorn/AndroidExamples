package com.lonelycorn.fragmentdemo;


import android.content.Context;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

/**
 * Created by cyan on 3/20/18.
 */

public class FirstFragment extends Fragment {

    private static final String TAG = "FirstFragment";

    @Override
    public void onAttach(Context context) {
        Log.i(TAG, "onAttach");
        super.onAttach(context);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);
    }


    // called when re-created from the backstack <-- what is this?
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        ViewGroup rootView = (ViewGroup) inflater.inflate(
                R.layout.fragment_first, container, false);

        return rootView;
    }


    @Override
    public void onActivityCreated(Bundle savedInstance) {
        Log.i(TAG, "onActivityCreated");
        super.onActivityCreated(savedInstance);
    }

    @Override
    public void onStart() {
        Log.i(TAG, "onStart");
        super.onStart();
    }

    // now it becomes visible


    @Override
    public void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();
    }

    // now it becomes active

    // DO STUFF HERE

    // now it is no longer active

    @Override
    public void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
    }

    // no it is no longer visible

    @Override
    public void onStop() {
        Log.i(TAG, "onStop");
        super.onStop();
    }

    @Override
    public void onDestroyView() {
        Log.i(TAG, "onDestroyView");
        super.onDestroyView();
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "onDestroy");
        super.onDestroy();
    }

    @Override
    public void onDetach() {
        Log.i(TAG, "onDetach");
        super.onDetach();
    }
}
