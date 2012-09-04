package org.paintown;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Environment;

import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.OrientationEventListener;
import android.view.WindowManager;

import android.hardware.*;
import android.content.res.Configuration;
import android.content.res.AssetManager;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.PixelFormat;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ActivityInfo;
import android.util.Log;

import java.lang.String;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.lang.Runnable;

import java.util.List;
import java.util.BitSet;
import java.util.ArrayList;

import java.io.File;
import java.io.InputStream;
import java.io.FileOutputStream;

import java.nio.ByteBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.*;

import org.liballeg.app.AllegroActivity;
import android.media.AudioManager;

public class PaintownActivity extends AllegroActivity {
   /* load allegro */
   static {
		/* FIXME: see if we can't load the allegro library name, or type from the manifest here */
      System.loadLibrary("OpenSLES");
      /*
      System.loadLibrary("allegro");
      System.loadLibrary("allegro_primitives");
      System.loadLibrary("allegro_image");
      System.loadLibrary("allegro_memfile");
      System.loadLibrary("allegro_font");
      System.loadLibrary("allegro_ttf");
      System.loadLibrary("allegro_audio");
      System.loadLibrary("allegro_acodec");
      */

      System.loadLibrary("allegro-debug");
      System.loadLibrary("allegro_primitives-debug");
      System.loadLibrary("allegro_image-debug");
      System.loadLibrary("allegro_memfile-debug");
      System.loadLibrary("allegro_font-debug");
      System.loadLibrary("allegro_ttf-debug");
      System.loadLibrary("allegro_audio-debug");
      System.loadLibrary("allegro_acodec-debug");
      System.loadLibrary("paintown");
   }

   public PaintownActivity(){
   }
}
