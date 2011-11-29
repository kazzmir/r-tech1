package org.libsdl.app;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.*;

import java.util.zip.*;
import java.io.File;
import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.StringReader;
import java.io.InputStream;

import android.content.res.AssetManager;
import android.content.res.AssetFileDescriptor;
import android.app.*;
import android.content.*;
import android.view.*;
import android.os.*;
import android.util.Log;
import android.graphics.*;
import android.text.method.*;
import android.text.*;
import android.media.*;
import android.hardware.*;
import android.content.*;

import android.widget.RelativeLayout;
import android.widget.LinearLayout;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.ProgressBar;

import android.util.AttributeSet;
import android.util.Xml;
import org.xmlpull.v1.XmlPullParser;

import java.lang.*;

public class SDLActivity extends Activity {

    // Main components
    private static SDLActivity mSingleton;
    private static SDLSurface mSurface;

    // Audio
    private static Thread mAudioThread;
    private static AudioTrack mAudioTrack;

    // Load the .so
    static {
        System.loadLibrary("SDL");
        System.loadLibrary("paintown");
    }

    public static String getDataDirectory(){
        return Environment.getExternalStorageDirectory().getAbsolutePath() + "/paintown";
    }

    /* copy the data bundled in assets to the external data directory */
    private void setupData(final Context context){
        final File root = new File(getDataDirectory());
        if (root.exists()){
            return;
        }
        
        /* horizontal progress bar */
        final ProgressBar progress = new ProgressBar(context, null, android.R.attr.progressBarStyleHorizontal);

        /* we are in a background thread so to get the bar to show up we
         * need to run it on the main UI thread
         */
        runOnUiThread(new Runnable(){
            public void run(){
                setContentView(loadingView(context, progress));
            }
        });

        Log.v("SDL", "Data directory doesn't exist, creating it: " + getDataDirectory());
        if (!root.mkdirs()){
            Log.v("SDL", "Unable to make data directory");
            return;
        }

        File user = new File(root, "user");
        if (!user.exists()){
            user.mkdirs();
        }

        unzip(root, "data.zip", context, progress);
    }

    /* gets the number of entries in a zip file.. but this is slow with an input stream */
    private int countZipFiles(InputStream stream) throws IOException {
        int total = 0;
        ZipInputStream zip = new ZipInputStream(stream);
        ZipEntry entry = zip.getNextEntry();
        while (entry != null){
            total += 1;
            entry = zip.getNextEntry();
        }
        zip.close();
        return total;
    }

    /* get the size of the entire zip file */
    private long zipSize(AssetManager assets, String file) throws IOException {
        AssetFileDescriptor descriptor = assets.openFd(file);
        long size = descriptor.getLength();
        descriptor.close();
        return size;
    }

    /* unzips a file from assets into the given root directory */
    private void unzip(File root, String file, Context context, final ProgressBar progress){
        Log.v("SDL", "Writing data to " + root.getAbsolutePath());
        try{
            AssetManager assets = context.getResources().getAssets();
            progress.setMax((int) zipSize(assets, file));
            ZipInputStream zip = new ZipInputStream(assets.open(file));

            ZipEntry entry = zip.getNextEntry();
            long count = 0;
            while (entry != null){
                String filename = entry.getName();
                if (entry.isDirectory()){
                    File directory = new File(root, filename);
                    directory.mkdirs();
                } else {
                    writeFile(new File(root, filename), entry.getSize(), zip);
                }
                
                count += entry.getSize();

                entry = zip.getNextEntry();
                final long xcount = count;

                /* update the progress bar */
                runOnUiThread(new Runnable(){
                    public void run(){
                        progress.setProgress((int) xcount);
                    }
                });
            }
            zip.close();
        } catch (IOException fail){
            Log.v("SDL", fail.toString());
        }
        Log.v("SDL", "Wrote data");
    }

    private void writeFile(File what, long size, ZipInputStream stream) throws IOException {
        byte[] buffer = new byte[1024];
        int count;
        BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(what));
        while ((count = stream.read(buffer, 0, buffer.length)) != -1){
            output.write(buffer, 0, count);
        }
        output.close();
    }

    // Setup
    protected void onCreate(Bundle savedInstanceState) {
        //Log.v("SDL", "onCreate()");
        super.onCreate(savedInstanceState);

        setContentView(welcomeView(getApplication()));
        
        // So we can call stuff from static callbacks
        mSingleton = this;

        new Thread(){
            public void run(){
                setupData(getApplication());
                runOnUiThread(new Runnable(){
                    public void run(){
                        setupSDL();
                    }
                });
            }
        }.start();
    }

    private View welcomeView(Context context){
        RelativeLayout layout = new RelativeLayout(context);
        RelativeLayout.LayoutParams textParams = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        textParams.addRule(RelativeLayout.CENTER_IN_PARENT);

        TextView text = new TextView(context);
        text.setId(800);
        text.setText("Starting Paintown..");
        layout.addView(text, textParams);
        return layout;
    }

    private void setupSDL(){
        mSurface = new SDLSurface(getApplication());
        setContentView(createView(mSurface));
        SurfaceHolder holder = mSurface.getHolder();
        holder.setType(SurfaceHolder.SURFACE_TYPE_GPU);
    }

    private View loadingView(Context context, ProgressBar progress){
        RelativeLayout layout = new RelativeLayout(context);
        RelativeLayout.LayoutParams textParams = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        textParams.addRule(RelativeLayout.CENTER_IN_PARENT);

        TextView text = new TextView(context);
        text.setId(800);
        text.setText("Installing Paintown data to " + getDataDirectory() + "/data");
        layout.addView(text, textParams);
        
        RelativeLayout.LayoutParams progressParams = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        progressParams.addRule(RelativeLayout.BELOW, text.getId());
        progressParams.addRule(RelativeLayout.CENTER_HORIZONTAL);
        layout.addView(progress, progressParams);
        return layout;
    }

    public static boolean isActionDown(MotionEvent event){
        int action = event.getAction();
        return action == MotionEvent.ACTION_DOWN;
        /*
        return action == MotionEvent.ACTION_DOWN ||
               action == MotionEvent.ACTION_POINTER_1_DOWN ||
               action == MotionEvent.ACTION_POINTER_2_DOWN ||
               action == MotionEvent.ACTION_POINTER_3_DOWN;
               */
    }

    public static boolean isActionUp(MotionEvent event){
        int action = event.getAction();
        return action == MotionEvent.ACTION_UP;
        /*
        return action == MotionEvent.ACTION_UP ||
               action == MotionEvent.ACTION_POINTER_1_UP ||
               action == MotionEvent.ACTION_POINTER_2_UP ||
               action == MotionEvent.ACTION_POINTER_3_UP;
               */
    }

    class OnScreenButton extends ImageView implements View.OnTouchListener {
        public OnScreenButton(Context context, int key, int resource){
            super(context);
            this.key = key;
            setImageResource(resource);
            setOnTouchListener(this);   
            setPadding(5, 5, 5, 5);
        }

        private int key;

        public boolean onTouch(View view, MotionEvent event) {
            int action = event.getAction();
            float x = event.getX();
            float y = event.getY();
            float p = event.getPressure();

            // Log.v("SDL", "button " + this.key + " at " + x + ", " + y + " action " + action);
            // Log.v("SDL", " button " + this.key);
            if (SDLActivity.isActionDown(event)){
                // Log.v("SDL", " down " + x + ", " + y + " action " + action);
                SDLActivity.onNativeKeyDown(this.key);
                return true;
            } else if (SDLActivity.isActionUp(event)){
                // Log.v("SDL", " up " + x + ", " + y + " action " + action);
                SDLActivity.onNativeKeyUp(this.key);
                return true;
            }

            return false;
        }
    }

    /* TODO: add more space between buttons */
    private View makeButtons(Context context){
        RelativeLayout group = new RelativeLayout(context);

        LinearLayout top = new LinearLayout(context);
        LinearLayout bottom = new LinearLayout(context);

        top.setId(500);
        bottom.setId(501);

        top.setOrientation(LinearLayout.HORIZONTAL);
        bottom.setOrientation(LinearLayout.HORIZONTAL);

        top.addView(new OnScreenButton(context, KeyEvent.KEYCODE_A, R.drawable.button1));
        top.addView(new OnScreenButton(context, KeyEvent.KEYCODE_S, R.drawable.button1));
        top.addView(new OnScreenButton(context, KeyEvent.KEYCODE_D, R.drawable.button1));
        
        bottom.addView(new OnScreenButton(context, KeyEvent.KEYCODE_Z, R.drawable.button2));
        bottom.addView(new OnScreenButton(context, KeyEvent.KEYCODE_X, R.drawable.button2));
        bottom.addView(new OnScreenButton(context, KeyEvent.KEYCODE_D, R.drawable.button2));
        
        RelativeLayout.LayoutParams paramsTop = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        paramsTop.addRule(RelativeLayout.ABOVE, bottom.getId());
        
        RelativeLayout.LayoutParams paramsBottom = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        paramsBottom.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);

        group.addView(top, paramsTop);
        group.addView(bottom, paramsBottom);

        return group;
    }

    private View makePad(Context context){
        LinearLayout group = new LinearLayout(context);
        group.setOrientation(LinearLayout.VERTICAL);

        class Dot extends ImageView {
            public Dot(Context context){
                super(context);
                setImageResource(R.drawable.dot);
            }
        }

        LinearLayout top = new LinearLayout(context);
        top.addView(new Dot(context));
        top.addView(new Dot(context));
        top.addView(new Dot(context));
        top.addView(new Dot(context));
        top.addView(new Dot(context));
        top.addView(new Dot(context));
        top.addView(new Dot(context));
        top.addView(new OnScreenButton(context, KeyEvent.KEYCODE_DPAD_UP, R.drawable.arrowup));
        top.addView(new Dot(context));

        LinearLayout middle = new LinearLayout(context);
        middle.addView(new OnScreenButton(context, KeyEvent.KEYCODE_DPAD_LEFT, R.drawable.arrowleft));
        middle.addView(new Dot(context));
        middle.addView(new OnScreenButton(context, KeyEvent.KEYCODE_DPAD_RIGHT, R.drawable.arrowright));

        LinearLayout bottom = new LinearLayout(context);
        bottom.addView(new Dot(context));
        bottom.addView(new Dot(context));
        bottom.addView(new Dot(context));
        bottom.addView(new Dot(context));
        bottom.addView(new Dot(context));
        bottom.addView(new Dot(context));
        bottom.addView(new Dot(context));
        bottom.addView(new OnScreenButton(context, KeyEvent.KEYCODE_DPAD_DOWN, R.drawable.arrowdown));
        bottom.addView(new Dot(context));

        group.addView(top);
        group.addView(middle);
        group.addView(bottom);

        return group;
    }
    
    private View makePad2(Context context){
        RelativeLayout group = new RelativeLayout(context);
        int id = 400;

        ImageView centerView = new ImageView(context);
        centerView.setImageResource(R.drawable.dot);
        centerView.setPadding(5, 5, 5, 5);
        centerView.setId(id); id += 1;
        RelativeLayout.LayoutParams center = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        center.addRule(RelativeLayout.CENTER_IN_PARENT);

        group.addView(centerView, center);

        RelativeLayout.LayoutParams up = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        up.addRule(RelativeLayout.ABOVE, centerView.getId());
        up.addRule(RelativeLayout.CENTER_HORIZONTAL);
        OnScreenButton upButton = new OnScreenButton(context, KeyEvent.KEYCODE_DPAD_UP, R.drawable.arrowup);
        upButton.setId(id); id += 1;
        group.addView(upButton, up);

        RelativeLayout.LayoutParams down = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        down.addRule(RelativeLayout.BELOW, centerView.getId());
        down.addRule(RelativeLayout.CENTER_HORIZONTAL);
        OnScreenButton downButton = new OnScreenButton(context, KeyEvent.KEYCODE_DPAD_DOWN, R.drawable.arrowdown);
        downButton.setId(id); id += 1;
        group.addView(downButton, down);

        RelativeLayout.LayoutParams left = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        left.addRule(RelativeLayout.LEFT_OF, centerView.getId());
        left.addRule(RelativeLayout.CENTER_VERTICAL);
        OnScreenButton leftButton = new OnScreenButton(context, KeyEvent.KEYCODE_DPAD_LEFT, R.drawable.arrowleft);
        leftButton.setId(id); id += 1;
        group.addView(leftButton, left);

        RelativeLayout.LayoutParams right = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        right.addRule(RelativeLayout.RIGHT_OF, centerView.getId());
        right.addRule(RelativeLayout.CENTER_VERTICAL);
        OnScreenButton rightButton = new OnScreenButton(context, KeyEvent.KEYCODE_DPAD_RIGHT, R.drawable.arrowright);
        rightButton.setId(id); id += 1;
        group.addView(rightButton, right);

        ImageView dotQ3 = new ImageView(context);
        dotQ3.setImageResource(R.drawable.dot);
        dotQ3.setPadding(5, 5, 5, 5);
        dotQ3.setId(id); id += 1;
        RelativeLayout.LayoutParams dotQ3Params = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        dotQ3Params.addRule(RelativeLayout.LEFT_OF, downButton.getId());
        dotQ3Params.addRule(RelativeLayout.BELOW, leftButton.getId());

        group.addView(dotQ3, dotQ3Params);
        
        return group;
    }

    /* sets up the d-pad, main game area, and the buttons*/
    private View createView(SDLSurface main){
        Context context = getApplication();
        main.setId(100);
        Log.v("SDL", "Surface id " + main.getId());
        RelativeLayout group = new RelativeLayout(context);

        /* main game always runs at 640, 480 */
        RelativeLayout.LayoutParams params0 = new RelativeLayout.LayoutParams(
                640, 480);
        params0.addRule(RelativeLayout.CENTER_IN_PARENT);
        group.addView(main, params0);

        View pad = makePad(context);
        pad.setId(101);
        Log.v("SDL", "Pad id " + pad.getId());
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        params.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        // params.addRule(RelativeLayout.CENTER_IN_PARENT);
        group.addView(pad, params);

        View buttons = makeButtons(context);
        buttons.setId(102);
        RelativeLayout.LayoutParams params1 = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        params1.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        params1.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        group.addView(buttons, params1);

        /* make sure the d-pad and buttons are in front of the actual game */
        group.bringChildToFront(pad);
        group.bringChildToFront(buttons);

        return group;
    }

    // Events
    protected void onPause() {
        //Log.v("SDL", "onPause()");
        super.onPause();
    }

    protected void onResume() {
        //Log.v("SDL", "onResume()");
        super.onResume();
    }

    // Messages from the SDLMain thread
    static int COMMAND_CHANGE_TITLE = 1;

    // Handler for the messages
    Handler commandHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (msg.arg1 == COMMAND_CHANGE_TITLE) {
                setTitle((String)msg.obj);
            }
        }
    };

    // Send a message from the SDLMain thread
    void sendCommand(int command, Object data) {
        Message msg = commandHandler.obtainMessage();
        msg.arg1 = command;
        msg.obj = data;
        commandHandler.sendMessage(msg);
    }

    // C functions we call
    public static native void nativeInit();
    public static native void nativeQuit();
    public static native void onNativeResize(int x, int y, int format);
    public static native void onNativeKeyDown(int keycode);
    public static native void onNativeKeyUp(int keycode);
    public static native void onNativeTouch(int action, float x, 
                                            float y, float p);
    public static native void onNativeAccel(float x, float y, float z);
    public static native void nativeRunAudioThread();

    // Set the SD card path
    public static native void setExternalLocation(String path);
    
    // Java functions called from C

    public static boolean createGLContext(int majorVersion, int minorVersion) {
        return mSurface.initEGL(majorVersion, minorVersion);
    }

    public static void flipBuffers() {
        mSurface.flipEGL();
    }

    public static void setActivityTitle(String title) {
        // Called from SDLMain() thread and can't directly affect the view
        mSingleton.sendCommand(COMMAND_CHANGE_TITLE, title);
    }

    public static Context getContext() {
        return mSingleton;
    }

    public static SDLActivity getActivity(){
        return mSingleton;
    }

    // Audio
    private static Object buf;
    
    public static Object audioInit(int sampleRate, boolean is16Bit, boolean isStereo, int desiredFrames) {
        int channelConfig = isStereo ? AudioFormat.CHANNEL_CONFIGURATION_STEREO : AudioFormat.CHANNEL_CONFIGURATION_MONO;
        int audioFormat = is16Bit ? AudioFormat.ENCODING_PCM_16BIT : AudioFormat.ENCODING_PCM_8BIT;
        int frameSize = (isStereo ? 2 : 1) * (is16Bit ? 2 : 1);
        
        Log.v("SDL", "SDL audio: wanted " + (isStereo ? "stereo" : "mono") + " " + (is16Bit ? "16-bit" : "8-bit") + " " + ((float)sampleRate / 1000f) + "kHz, " + desiredFrames + " frames buffer");
        
        // Let the user pick a larger buffer if they really want -- but ye
        // gods they probably shouldn't, the minimums are horrifyingly high
        // latency already
        desiredFrames = Math.max(desiredFrames, (AudioTrack.getMinBufferSize(sampleRate, channelConfig, audioFormat) + frameSize - 1) / frameSize);
        
        mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
                channelConfig, audioFormat, desiredFrames * frameSize, AudioTrack.MODE_STREAM);
        
        audioStartThread();
        
        Log.v("SDL", "SDL audio: got " + ((mAudioTrack.getChannelCount() >= 2) ? "stereo" : "mono") + " " + ((mAudioTrack.getAudioFormat() == AudioFormat.ENCODING_PCM_16BIT) ? "16-bit" : "8-bit") + " " + ((float)mAudioTrack.getSampleRate() / 1000f) + "kHz, " + desiredFrames + " frames buffer");
        
        if (is16Bit) {
            buf = new short[desiredFrames * (isStereo ? 2 : 1)];
        } else {
            buf = new byte[desiredFrames * (isStereo ? 2 : 1)]; 
        }
        return buf;
    }
    
    public static void audioStartThread() {
        mAudioThread = new Thread(new Runnable() {
            public void run() {
                mAudioTrack.play();
                nativeRunAudioThread();
            }
        });
        
        // I'd take REALTIME if I could get it!
        mAudioThread.setPriority(Thread.MAX_PRIORITY);
        mAudioThread.start();
    }
    
    public static void audioWriteShortBuffer(short[] buffer) {
        for (int i = 0; i < buffer.length; ) {
            int result = mAudioTrack.write(buffer, i, buffer.length - i);
            if (result > 0) {
                i += result;
            } else if (result == 0) {
                try {
                    Thread.sleep(1);
                } catch(InterruptedException e) {
                    // Nom nom
                }
            } else {
                Log.w("SDL", "SDL audio: error return from write(short)");
                return;
            }
        }
    }
    
    public static void audioWriteByteBuffer(byte[] buffer) {
        for (int i = 0; i < buffer.length; ) {
            int result = mAudioTrack.write(buffer, i, buffer.length - i);
            if (result > 0) {
                i += result;
            } else if (result == 0) {
                try {
                    Thread.sleep(1);
                } catch(InterruptedException e) {
                    // Nom nom
                }
            } else {
                Log.w("SDL", "SDL audio: error return from write(short)");
                return;
            }
        }
    }

    public static void audioQuit() {
        if (mAudioThread != null) {
            try {
                mAudioThread.join();
            } catch(Exception e) {
                Log.v("SDL", "Problem stopping audio thread: " + e);
            }
            mAudioThread = null;

            //Log.v("SDL", "Finished waiting for audio thread");
        }

        if (mAudioTrack != null) {
            mAudioTrack.stop();
            mAudioTrack = null;
        }
    }
}

/**
    Simple nativeInit() runnable
*/
class SDLMain implements Runnable {
    public void run() {
        // Runs SDL_main()
        SDLActivity.setExternalLocation(SDLActivity.getDataDirectory());
        SDLActivity.nativeInit();
        Log.v("SDL", "SDL thread terminated");
        SDLActivity.getActivity().finish();
    }
}


/**
    SDLSurface. This is what we draw on, so we need to know when it's created
    in order to do anything useful. 

    Because of this, that's where we set up the SDL thread
*/
class SDLSurface extends SurfaceView implements SurfaceHolder.Callback, 
    View.OnKeyListener, View.OnTouchListener, SensorEventListener  {

    // This is what SDL runs in. It invokes SDL_main(), eventually
    private Thread mSDLThread;    
    
    // EGL private objects
    private EGLContext  mEGLContext;
    private EGLSurface  mEGLSurface;
    private EGLDisplay  mEGLDisplay;

    // Sensors
    private static SensorManager mSensorManager;

    // Startup    
    public SDLSurface(Context context) {
        super(context);
        getHolder().addCallback(this); 
    
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
        setOnKeyListener(this); 
        setOnTouchListener(this);   

        mSensorManager = (SensorManager)context.getSystemService("sensor");  
    }

    // Called when we have a valid drawing surface
    public void surfaceCreated(SurfaceHolder holder) {
        //Log.v("SDL", "surfaceCreated()");

        enableSensor(Sensor.TYPE_ACCELEROMETER, true);
    }

    // Called when we lose the surface
    public void surfaceDestroyed(SurfaceHolder holder) {
        //Log.v("SDL", "surfaceDestroyed()");

        // Send a quit message to the application
        SDLActivity.nativeQuit();

        // Now wait for the SDL thread to quit
        if (mSDLThread != null) {
            try {
                mSDLThread.join();
            } catch(Exception e) {
                Log.v("SDL", "Problem stopping thread: " + e);
            }
            mSDLThread = null;

            //Log.v("SDL", "Finished waiting for SDL thread");
        }

        enableSensor(Sensor.TYPE_ACCELEROMETER, false);
    }

    // Called when the surface is resized
    public void surfaceChanged(SurfaceHolder holder,
                               int format, int width, int height) {
        //Log.v("SDL", "surfaceChanged()");
        Log.v("SDL", "Surface changed: format " + format + " width " + width + " height " + height);
        
        /* Force screen parameters to be a minimum size */
        if (width < 640){
            width = 640;
        }
        if (height < 480){
            height = 480;
        }

        int sdlFormat = 0x85151002; // SDL_PIXELFORMAT_RGB565 by default
        switch (format) {
        case PixelFormat.A_8:
            Log.v("SDL", "pixel format A_8");
            break;
        case PixelFormat.LA_88:
            Log.v("SDL", "pixel format LA_88");
            break;
        case PixelFormat.L_8:
            Log.v("SDL", "pixel format L_8");
            break;
        case PixelFormat.RGBA_4444:
            Log.v("SDL", "pixel format RGBA_4444");
            sdlFormat = 0x85421002; // SDL_PIXELFORMAT_RGBA4444
            break;
        case PixelFormat.RGBA_5551:
            Log.v("SDL", "pixel format RGBA_5551");
            sdlFormat = 0x85441002; // SDL_PIXELFORMAT_RGBA5551
            break;
        case PixelFormat.RGBA_8888:
            Log.v("SDL", "pixel format RGBA_8888");
            sdlFormat = 0x86462004; // SDL_PIXELFORMAT_RGBA8888
            break;
        case PixelFormat.RGBX_8888:
            Log.v("SDL", "pixel format RGBX_8888");
            sdlFormat = 0x86262004; // SDL_PIXELFORMAT_RGBX8888
            break;
        case PixelFormat.RGB_332:
            Log.v("SDL", "pixel format RGB_332");
            sdlFormat = 0x84110801; // SDL_PIXELFORMAT_RGB332
            break;
        case PixelFormat.RGB_565:
            Log.v("SDL", "pixel format RGB_565");
            sdlFormat = 0x85151002; // SDL_PIXELFORMAT_RGB565
            break;
        case PixelFormat.RGB_888:
            Log.v("SDL", "pixel format RGB_888");
            // Not sure this is right, maybe SDL_PIXELFORMAT_RGB24 instead?
            sdlFormat = 0x86161804; // SDL_PIXELFORMAT_RGB888
            break;
        default:
            Log.v("SDL", "pixel format unknown " + format);
            break;
        }
        SDLActivity.onNativeResize(width, height, sdlFormat);

        // Now start up the C app thread
        if (mSDLThread == null) {
            mSDLThread = new Thread(new SDLMain(), "Paintown"); 
            mSDLThread.start();       
        }
    }

    public void onDraw(Canvas canvas){
        /* draw the touch screen here */
    }

    // EGL functions
    public boolean initEGL(int majorVersion, int minorVersion) {
        Log.v("SDL", "Starting up OpenGL ES " + majorVersion + "." + minorVersion);

        try {
            EGL10 egl = (EGL10)EGLContext.getEGL();

            EGLDisplay dpy = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

            int[] version = new int[2];
            egl.eglInitialize(dpy, version);

            int EGL_OPENGL_ES_BIT = 1;
            int EGL_OPENGL_ES2_BIT = 4;
            int renderableType = 0;
            if (majorVersion == 2) {
                renderableType = EGL_OPENGL_ES2_BIT;
            } else if (majorVersion == 1) {
                renderableType = EGL_OPENGL_ES_BIT;
            }
            int[] configSpec = {
                //EGL10.EGL_DEPTH_SIZE,   16,
                EGL10.EGL_RENDERABLE_TYPE, renderableType,
                EGL10.EGL_NONE
            };
            EGLConfig[] configs = new EGLConfig[1];
            int[] num_config = new int[1];
            if (!egl.eglChooseConfig(dpy, configSpec, configs, 1, num_config) || num_config[0] == 0) {
                Log.e("SDL", "No EGL config available");
                return false;
            }
            EGLConfig config = configs[0];

            EGLContext ctx = egl.eglCreateContext(dpy, config, EGL10.EGL_NO_CONTEXT, null);
            if (ctx == EGL10.EGL_NO_CONTEXT) {
                Log.e("SDL", "Couldn't create context");
                return false;
            }

            EGLSurface surface = egl.eglCreateWindowSurface(dpy, config, this, null);
            if (surface == EGL10.EGL_NO_SURFACE) {
                Log.e("SDL", "Couldn't create surface");
                return false;
            }

            if (!egl.eglMakeCurrent(dpy, surface, surface, ctx)) {
                Log.e("SDL", "Couldn't make context current");
                return false;
            }

            mEGLContext = ctx;
            mEGLDisplay = dpy;
            mEGLSurface = surface;

        } catch(Exception e) {
            Log.v("SDL", e + "");
            for (StackTraceElement s : e.getStackTrace()) {
                Log.v("SDL", s.toString());
            }
        }

        return true;
    }

    // EGL buffer flip
    public void flipEGL() {
        try {
            EGL10 egl = (EGL10)EGLContext.getEGL();

            egl.eglWaitNative(EGL10.EGL_NATIVE_RENDERABLE, null);

            // drawing here

            egl.eglWaitGL();

            egl.eglSwapBuffers(mEGLDisplay, mEGLSurface);

            
        } catch(Exception e) {
            Log.v("SDL", "flipEGL(): " + e);
            for (StackTraceElement s : e.getStackTrace()) {
                Log.v("SDL", s.toString());
            }
        }
    }

    // Key events
    public boolean onKey(View v, int keyCode, KeyEvent event) {

        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            //Log.v("SDL", "key down: " + keyCode);
            SDLActivity.onNativeKeyDown(keyCode);
            return true;
        }
        else if (event.getAction() == KeyEvent.ACTION_UP) {
            //Log.v("SDL", "key up: " + keyCode);
            SDLActivity.onNativeKeyUp(keyCode);
            return true;
        }
        
        return false;
    }

    public boolean inScope(float x, float y){
        Rect out = new Rect();
        getGlobalVisibleRect(out);
        return out.contains((int) x, (int) y);
    }

    // Touch events
    public boolean onTouch(View view, MotionEvent event) {
        int action = event.getAction();
        float x = event.getX();
        float y = event.getY();
        float p = event.getPressure();

        // Log.v("SDL", "touch " + x + ", " + y);

        if (SDLActivity.isActionDown(event)){
            SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_ENTER);
        } else if (SDLActivity.isActionUp(event)){
            SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_ENTER);
        }

        // TODO: Anything else we need to pass?        
        SDLActivity.onNativeTouch(action, x, y, p);
        return true;
    }

    // Sensor events
    public void enableSensor(int sensortype, boolean enabled) {
        // TODO: This uses getDefaultSensor - what if we have >1 accels?
        if (enabled) {
            mSensorManager.registerListener(this, 
                            mSensorManager.getDefaultSensor(sensortype), 
                            SensorManager.SENSOR_DELAY_GAME, null);
        } else {
            mSensorManager.unregisterListener(this, 
                            mSensorManager.getDefaultSensor(sensortype));
        }
    }
    
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO
    }

    public void onSensorChanged(SensorEvent event) {
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            SDLActivity.onNativeAccel(event.values[0],
                                           event.values[1],
                                           event.values[2]);
        }
    }

}

