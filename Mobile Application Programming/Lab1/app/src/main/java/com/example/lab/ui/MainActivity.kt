package com.example.lab.ui

import android.Manifest
import android.content.pm.PackageManager
import android.location.Geocoder
import android.os.Bundle
import android.view.GestureDetector
import android.view.MotionEvent
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import com.example.lab.R
import com.example.lab.data.Calculator
import com.google.android.gms.location.FusedLocationProviderClient
import com.google.android.gms.location.LocationServices
import java.util.Locale
import kotlin.math.abs

class MainActivity : AppCompatActivity() {

    private lateinit var calculator: Calculator
    private lateinit var tvResult: TextView
    private lateinit var gestureDetector: GestureDetector
    private lateinit var fusedLocationClient: FusedLocationProviderClient

    override fun onCreate(savedInstanceState: Bundle?) {
        installSplashScreen()
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        calculator = Calculator()
        tvResult = findViewById(R.id.tvResult)
        fusedLocationClient = LocationServices.getFusedLocationProviderClient(this)

        setupButtons()
        setupGestures()
    }

    private fun setupButtons() {
        setNumberButton(R.id.btn0, "0")
        setNumberButton(R.id.btn1, "1")
        setNumberButton(R.id.btn2, "2")
        setNumberButton(R.id.btn3, "3")
        setNumberButton(R.id.btn4, "4")
        setNumberButton(R.id.btn5, "5")
        setNumberButton(R.id.btn6, "6")
        setNumberButton(R.id.btn7, "7")
        setNumberButton(R.id.btn8, "8")
        setNumberButton(R.id.btn9, "9")
        setNumberButton(R.id.btnDot, ".")

        setOperationButton(R.id.btnPlus, "+")
        setOperationButton(R.id.btnMinus, "-")
        setOperationButton(R.id.btnMultiply, "*")
        setOperationButton(R.id.btnDivide, "/")

        findViewById<Button>(R.id.btnEquals).setOnClickListener {
            val result = calculator.calculate(tvResult.text.toString())
            tvResult.text = result
        }

        findViewById<Button>(R.id.btnClear).setOnClickListener {
            tvResult.text = calculator.clear()
        }

        findViewById<Button>(R.id.btnBackspace).setOnClickListener {
            tvResult.text = calculator.backspace(tvResult.text.toString())
        }
    }

    private fun setNumberButton(buttonId: Int, digit: String) {
        findViewById<Button>(buttonId).setOnClickListener {
            val newText = calculator.appendDigit(digit, tvResult.text.toString())
            tvResult.text = newText
        }
    }

    private fun setOperationButton(buttonId: Int, op: String) {
        findViewById<Button>(buttonId).setOnClickListener {
            calculator.setOperation(op, tvResult.text.toString())
        }
    }

    private fun setupGestures() {
        gestureDetector = GestureDetector(this, object : GestureDetector.SimpleOnGestureListener() {
            override fun onFling(
                e1: MotionEvent?,
                e2: MotionEvent,
                velocityX: Float,
                velocityY: Float
            ): Boolean {
                if (e1 == null) return false
                val diffX = e2.x - e1.x
                val diffY = e2.y - e1.y
                if (abs(diffX) > abs(diffY)) {
                    if (abs(diffX) > 100 && abs(velocityX) > 100) {
                        if (diffX > 0) {
                            onSwipeRight()
                        }
                    }
                }
                return super.onFling(e1, e2, velocityX, velocityY)
            }
        })

        findViewById<android.view.View>(android.R.id.content).setOnTouchListener { _, event ->
            gestureDetector.onTouchEvent(event)
            true
        }
    }

    private fun onSwipeRight() {
        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.ACCESS_FINE_LOCATION
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            ActivityCompat.requestPermissions(
                this,
                arrayOf(Manifest.permission.ACCESS_FINE_LOCATION),
                1001
            )
            return
        }

        fusedLocationClient.lastLocation.addOnSuccessListener { location ->
            if (location != null) {
                val geocoder = Geocoder(this, Locale.getDefault())
                try {
                    val addresses = geocoder.getFromLocation(location.latitude, location.longitude, 1)
                    if (addresses != null && addresses.isNotEmpty()) {
                        val city = addresses[0].locality ?: "Unknown City"
                        val country = addresses[0].countryName ?: "Unknown Country"
                        val fullLocation = "$city, $country\nLat:${location.latitude} Lon:${location.longitude}"
                        
                        Toast.makeText(this, fullLocation, Toast.LENGTH_LONG).show()
                        tvResult.text = fullLocation
                    } else {
                        val basicLoc = "Lat:${location.latitude} Lon:${location.longitude}"
                        tvResult.text = basicLoc
                    }
                } catch (e: Exception) {
                    val basicLoc = "Lat:${location.latitude} Lon:${location.longitude}"
                    tvResult.text = basicLoc
                    Toast.makeText(this, "Network error. Showing coords only.", Toast.LENGTH_SHORT).show()
                }
            } else {
                Toast.makeText(this, "Location not found. Enable GPS.", Toast.LENGTH_SHORT).show()
            }
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == 1001 && grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            onSwipeRight()
        }
    }
}
