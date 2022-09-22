package com.nooskewl.m3;

import java.io.InputStream;
import java.util.regex.Pattern;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.webkit.WebView;

public class French_Manual_Viewer_Activity extends Activity {
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		String text = "";

		try {
			InputStream f = getAssets().open("manual_french.html");
			if (f != null) {
				int p = 0;
				while (true) {
					int available = f.available();
					if (available <= 0) {
						break;
					}
					byte[] b = new byte[available];
					f.read(b, p, available);
					text += new String(b);
				}

				// Android doesn't support style blocks, so remove them.
				Pattern pattern = Pattern.compile("<style>.*?<\\/style>", Pattern.DOTALL);
				text = pattern.matcher(text).replaceAll("");
				// The title just adds a repetitive line at the top, remove it.
				Pattern pattern2 = Pattern.compile("<head>.*?<\\/head>", Pattern.DOTALL);
				text = pattern2.matcher(text).replaceAll("");

				WebView webview = new WebView(this);
				webview.loadData(text, "text/html", null);

				setContentView(webview);

				Intent i = new Intent();
				i.putExtra("MESSAGE", "OK");
				setResult(RESULT_OK, i);
			}
		}
		catch (Exception e) {
			Log.d("MoRPG3", e.toString());
			text = "";
		}

		if (text == "") {
			Intent i = new Intent();
			i.putExtra("MESSAGE", "FAIL");
			setResult(RESULT_CANCELED, i);
		 
			if (android.os.Build.VERSION.SDK_INT >= 21) {
				finishAndRemoveTask();
			}
			else {
				finish();
			}
		}
	}
}
