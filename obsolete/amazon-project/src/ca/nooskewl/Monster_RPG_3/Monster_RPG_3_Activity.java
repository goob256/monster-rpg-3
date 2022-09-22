package ca.nooskewl.Monster_RPG_3;

import java.io.File;
import java.io.FileFilter;
import java.util.Locale;
import java.nio.file.Path;
import java.nio.file.PathMatcher;
import java.nio.file.FileSystems;
import java.util.Vector;
import java.util.EnumSet;

import android.os.Bundle;
import android.os.Build;
import android.content.Context;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Color;
import android.net.Uri;
import android.os.Vibrator;
import android.os.VibrationEffect;
import android.util.Base64;
import android.util.Log;
import android.view.Surface;
import com.amazon.ags.api.*;
import com.amazon.ags.api.achievements.*;
import com.amazon.ags.api.overlay.PopUpLocation;
import com.amazon.ags.api.whispersync.GameDataMap;
import com.amazon.ags.api.whispersync.model.SyncableString;

import org.libsdl.app.SDLActivity; 

import ca.nooskewl.Monster_RPG_3.License_Viewer_Activity;

public class Monster_RPG_3_Activity extends SDLActivity implements AGResponseCallback<RequestResponse>
{
	final static int LICENSE_REQUEST = 1;
	final static int MANUAL_REQUEST = 1;

	native static void resume_after_showing_license();
	native static void resume_after_showing_manual();
	native static void resume_after_showing_achievements();
	native static void pause();
	native static void resume();

	// This is so the screen is never cleared pure black, only shim::black (r:35, g:30, b:60)
	static boolean paused = false;

	int initialize_success = -1;

	//reference to the agsClient
	AmazonGamesClient agsClient;

	AmazonGamesCallback callback = new AmazonGamesCallback() {
		@Override
		public void onServiceNotReady(AmazonGamesStatus status) {
			//unable to use service
			Log.d("MoRPG3", "GameCircle not initialised: " + status.toString());
			agsClient = null;
			gameDataMap = null;
			initialize_success = 0;
		}
		@Override
		public void onServiceReady(AmazonGamesClient amazonGamesClient) {
			Log.d("MoRPG3", "GameCircle initialised!");
			agsClient = amazonGamesClient;
			//ready to use GameCircle
			agsClient.setPopUpLocation(PopUpLocation.TOP_CENTER);
			gameDataMap = AmazonGamesClient.getWhispersyncClient().getGameData();
			initialize_success = 1;
		}
	};

	//list of features your game uses (in this example, achievements and leaderboards)
	EnumSet<AmazonGamesFeature> myGameFeatures = EnumSet.of(AmazonGamesFeature.Achievements).of(AmazonGamesFeature.Whispersync);

	GameDataMap gameDataMap;

	@Override
	public void onCreate(Bundle savedInstance)
	{
		super.onCreate(savedInstance);
	}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		if (requestCode == LICENSE_REQUEST) {
			if (data != null) {
				if (resultCode == RESULT_OK && data.getExtras().getString("MESSAGE").equals("OK")) {
					show_license_result = 0;
				}
				else if (resultCode == RESULT_CANCELED && data.getExtras().getString("MESSAGE").equals("FAIL")) {
					show_license_result = 1;
				}
				else {
					show_license_result = 1;
				}
			}
			else {
				show_license_result = 1;
			}

			resume_after_showing_license();
		}
		else if (requestCode == MANUAL_REQUEST) {
			if (data != null) {
				if (resultCode == RESULT_OK && data.getExtras().getString("MESSAGE").equals("OK")) {
					show_manual_result = 0;
				}
				else if (resultCode == RESULT_CANCELED && data.getExtras().getString("MESSAGE").equals("FAIL")) {
					show_manual_result = 1;
				}
				else {
					show_manual_result = 1;
				}
			}
			else {
				show_manual_result = 1;
			}

			resume_after_showing_manual();
		}
	}

	@Override
	public void onStop()
	{
		super.onStop();
		Runnable runnable = new Runnable() {
			public void run() {
					mSurface.setBackgroundColor(Color.rgb(35, 30, 60)); // clear to "our" black instead instead of pure black... unset below in start_draw
				}
			};
		runOnUiThread(runnable);
		pause();
	}

	@Override
	public void onPause() {
		super.onPause();
		if (agsClient != null) {
			agsClient.release();
		}
	}
	
	@Override
	public void onRestart()
	{
		super.onRestart();
		resume();
	}

	@Override
	public void onResume()
	{
		super.onResume();
		initialize_success = -1;
		AmazonGamesClient.initialize(this, callback, myGameFeatures);
	}

	@Override
	public void onPostResume()
	{
		super.onPostResume();
		paused = true;
	}

	public void logString(String s)
	{
		Log.d("MoRPG3", s);
	}

	public String getAppdataDir()
	{
		return getFilesDir().getAbsolutePath();
	}
	
	public String getSDCardDir()
	{
		File f = getExternalFilesDir(null);
		if (f != null) {
			return f.getAbsolutePath();
		}
		else {
			return getFilesDir().getAbsolutePath();
		}
	}

	static int show_license_result;

	public void showLicense()
	{
		show_license_result = -1;
		Intent intent = new Intent(this, License_Viewer_Activity.class);
		startActivityForResult(intent, LICENSE_REQUEST);
	}

	static int show_manual_result;

	public void showManual(String language)
	{
		if (language.equals("french")) {
			show_manual_result = -1;
			Intent intent = new Intent(this, French_Manual_Viewer_Activity.class);
			startActivityForResult(intent, MANUAL_REQUEST);
		}
		else if (language.equals("spanish")) {
			show_manual_result = -1;
			Intent intent = new Intent(this, Spanish_Manual_Viewer_Activity.class);
			startActivityForResult(intent, MANUAL_REQUEST);
		}
		else {
			show_manual_result = -1;
			Intent intent = new Intent(this, English_Manual_Viewer_Activity.class);
			startActivityForResult(intent, MANUAL_REQUEST);
		}
	}

	public int getShowLicenseResult()
	{
		return show_license_result;
	}

	public int getShowManualResult()
	{
		return show_manual_result;
	}

	/*
	public void openURL(String url)
	{
		Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		startActivity(intent);
	}
	*/

	public void rumble(int milliseconds)
	{
		Vibrator v = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
		if (v != null && v.hasVibrator()) {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
				v.vibrate(VibrationEffect.createOneShot(milliseconds, VibrationEffect.DEFAULT_AMPLITUDE));
			}
			else {
				v.vibrate(milliseconds);
			}
		}
	}

	public boolean has_touchscreen()
	{
		return getPackageManager().hasSystemFeature("android.hardware.touchscreen");
	}

	public boolean has_vibrator()
	{
		Vibrator v = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
		if (v != null) {
			return v.hasVibrator();
		}
		else {
			return false;
		}
	}

	public void start_draw()
	{
		if (paused) {
			Runnable runnable = new Runnable() {
				public void run() {
						mSurface.setBackgroundColor(Color.TRANSPARENT);
					}
				};
			runOnUiThread(runnable);
			paused = false;
		}
	}
	
	public String get_android_language()
	{
		return Locale.getDefault().getLanguage();
	}

	private static File[] list_dir_files = null;

	public void list_dir_start(String path)
	{
		try {
			int slash = path.lastIndexOf('/');
			final String glob = path.substring(slash+1).replace("*", ".*"); // +1 works even if not found (-1+1 == 0)
			String dir = path.substring(0, slash);
			File f = new File(dir);
			list_dir_files = f.listFiles(new FileFilter() {
				public boolean accept(File f)
				{
					try {
						if (f.getName().matches(glob)) {
							return true;
						}
						else {
							return false;
						}
					}
					catch (Exception e) {
						Log.d("MoRPG3", "list_dir_start FileFilter throwing " + e.getMessage());
						return false;
					}
				}
			});
		}
		catch (Exception e) {
			list_dir_files = null;
			Log.d("MoRPG3", "list_dir_start throwing " + e.getMessage());
		}
	}

	public String list_dir_next()
	{
		if (list_dir_files == null) {
			return "";
		}
		else if (list_dir_files.length == 0) {
			list_dir_files = null;
			return "";
		}
		else {
			File f = list_dir_files[0];
			String name = f.getName();
			if (list_dir_files.length == 1) {
				list_dir_files = null;
			}
			else {
				File[] new_list = new File[list_dir_files.length-1];
				for (int i = 1; i < list_dir_files.length; i++) {
					new_list[i-1] = list_dir_files[i];
				}
				list_dir_files = new_list;
			}
			return name;
		}
	}
	
	public void achieve(int id) {
		if (agsClient != null) {
			AchievementsClient acClient = agsClient.getAchievementsClient();
			if (acClient != null) {
				AGResponseHandle<UpdateProgressResponse> handle = acClient.updateProgress(Integer.toString(id+1), 100.0f);
			}
		}
	}

	public boolean show_achievements()
	{
		Log.d("MoRPG3", "in show_achievements");
		if (agsClient != null) {
			AchievementsClient acClient = agsClient.getAchievementsClient();
			if (acClient != null) {
				Log.d("MoRPG3", "Showing achievements...");
				AGResponseHandle<RequestResponse> h = acClient.showAchievementsOverlay();
				h.setCallback(this);
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}

	public void onComplete(RequestResponse r) {
		resume_after_showing_achievements();
	}

	public int initialised()
	{
		return initialize_success;
	}
	
	public long cloud_date(String name)
	{
		if (agsClient.getPlayerClient().isSignedIn() == false) {
			return -1;
		}

		SyncableString ss = gameDataMap.getLatestString(name.replace(".dat", ".txt"));
		if (ss.isSet()) {
			String s = ss.getValue();
			if (s.equals("")) {
				return -1;
			}
			else {
				long l = Long.parseLong(s);
				return l;
			}
		}
		else {
			return -1;
		}
	}

	// Delete is not available in Play Services 6.5.87
	public boolean cloud_delete(String name)
	{
		if (agsClient.getPlayerClient().isSignedIn() == false) {
			return false;
		}

		SyncableString s = gameDataMap.getLatestString(name);
		s.set("");
		return true;
	}

	public byte[] cloud_read(String name)
	{
		if (agsClient.getPlayerClient().isSignedIn() == false) {
			return null;
		}

		SyncableString ss = gameDataMap.getLatestString(name);
		if (ss.isSet()) {
			String s = ss.getValue();
			if (s.equals("")) {
				return null;
			}
			else {
				return Base64.decode(s, Base64.DEFAULT);
			}
		}
		else {
			return null;
		}
	}

	public boolean cloud_save(String name, byte[] bytes)
	{
		if (agsClient.getPlayerClient().isSignedIn() == false) {
			return false;
		}

		SyncableString ss = gameDataMap.getLatestString(name);
		if (name.contains(".dat")) {
			ss.set(new String(Base64.encodeToString(bytes, Base64.DEFAULT)));
		}
		else {
			ss.set(new String(bytes));
		}
		return true;
	}

	int errcode = 0;
	String errmsg;

	public int cloud_get_error_code()
	{
		if (agsClient == null) {
			return 1;
		}
		else if (gameDataMap == null) {
			return 2;
		}
		else if (agsClient.getPlayerClient().isSignedIn() == false) {
			return 3;
		}
		int ret = errcode;
		errcode = 0;
		return ret;
	}

	public String amazon_get_error_message()
	{
		return errmsg;
	}

	public void cloud_synchronise()
	{
		if (agsClient.getPlayerClient().isSignedIn() == false) {
			return;
		}

		AmazonGamesClient.getWhispersyncClient().synchronize();
	}

	public boolean is_fire_tv()
	{
		if (getPackageManager().hasSystemFeature("amazon.hardware.fire_tv")) {
			return true;
		}
		return false;
	}
}
