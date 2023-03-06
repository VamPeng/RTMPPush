package com.vam.vpusher

import android.Manifest
import android.hardware.Camera.CameraInfo
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.View
import com.vam.vpusher.databinding.ActivityMainBinding

fun View.click(block: (view: View) -> Unit) {
    setOnClickListener(block)
}

fun Any.tag(): String = this::class.java.simpleName
fun Any.logI(text: String) {
    Log.i(tag(), text)
}

fun Any.logE(tag: String, text: String) {
    Log.e(tag(), text)
}

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val livePusher by lazy {
        LivePusher(
            this,
            720,
            1080,
            800_000,
            20,
            CameraInfo.CAMERA_FACING_FRONT
        )
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        requestPermissions(
            arrayOf(
                Manifest.permission.CAMERA,
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.RECORD_AUDIO
            ), 101
        )

        initView()

        livePusher.setPreviewDisplay(binding.mainSurfaceView.holder)
    }

    private fun initView() {
        binding.mainStart.click { startLive() }
        binding.mainStop.click { stopLive() }
        binding.mainSwitch.click { switchCamera() }
    }

    private fun switchCamera() {
        livePusher.switchCamera()
    }

    private fun startLive() {
        val url = "rtmp://192.168.2.101:1935/myapp"
//        val url = "rtmp://127.0.0.1:1935/myapp";
        livePusher.startLive(url)
    }

    private fun stopLive() {

    }


    /**
     * A native method that is implemented by the 'vpusher' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

}