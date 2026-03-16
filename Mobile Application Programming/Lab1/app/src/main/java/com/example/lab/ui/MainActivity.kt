package com.example.lab.ui

import android.Manifest
import android.content.pm.PackageManager
import android.graphics.Color
import android.location.Geocoder
import android.os.Build
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
import com.google.android.material.bottomsheet.BottomSheetDialog
import com.google.firebase.firestore.FirebaseFirestore
import com.google.firebase.firestore.Query
import com.google.firebase.remoteconfig.FirebaseRemoteConfig
import com.google.firebase.remoteconfig.FirebaseRemoteConfigSettings
import java.util.Locale
import kotlin.math.abs

class MainActivity : AppCompatActivity() {

    private lateinit var calculator: Calculator
    private lateinit var tvResult: TextView
    private lateinit var gestureDetector: GestureDetector
    private lateinit var fusedLocationClient: FusedLocationProviderClient
    
    private val db = FirebaseFirestore.getInstance()
    private val remoteConfig = FirebaseRemoteConfig.getInstance()
    
    private var cloudAccentColor = "#43A047"
    private var isLightTheme = false

    override fun onCreate(savedInstanceState: Bundle?) {
        installSplashScreen()
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        calculator = Calculator()
        tvResult = findViewById(R.id.tvResult)
        fusedLocationClient = LocationServices.getFusedLocationProviderClient(this)

        setupButtons()
        setupGestures()
        setupRemoteConfig()
        loadSavedTheme()
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.POST_NOTIFICATIONS), 1002)
        }
    }

    override fun dispatchTouchEvent(ev: MotionEvent?): Boolean {
        ev?.let { gestureDetector.onTouchEvent(it) }
        return super.dispatchTouchEvent(ev)
    }

    private fun loadSavedTheme() {
        db.collection("settings").document("theme").get().addOnSuccessListener { doc ->
            val mode = doc.getString("theme_mode")
            isLightTheme = mode == "light"
            applyThemeColors(cloudAccentColor, isLightTheme)
        }
    }

    private fun toggleTheme() {
        isLightTheme = !isLightTheme
        applyThemeColors(cloudAccentColor, isLightTheme)
        
        val mode = if (isLightTheme) "light" else "dark"
        db.collection("settings").document("theme").set(mapOf("theme_mode" to mode))
        Toast.makeText(this, if (isLightTheme) "Light Mode" else "Dark Mode", Toast.LENGTH_SHORT).show()
    }

    private fun setupRemoteConfig() {
        val configSettings = FirebaseRemoteConfigSettings.Builder()
            .setMinimumFetchIntervalInSeconds(0)
            .build()
        remoteConfig.setConfigSettingsAsync(configSettings)
        
        remoteConfig.setDefaultsAsync(mapOf("button_accent_color" to "#43A047"))

        remoteConfig.fetchAndActivate().addOnCompleteListener { task ->
            if (task.isSuccessful) {
                cloudAccentColor = remoteConfig.getString("button_accent_color")
                applyThemeColors(cloudAccentColor, isLightTheme)
            }
        }
    }

    private fun applyThemeColors(accentColorStr: String, isLight: Boolean) {
        try {
            val accentColor = Color.parseColor(accentColorStr)
            window.statusBarColor = Color.BLACK
            
            val bgColor = if (isLight) Color.parseColor("#F5F5F5") else Color.BLACK
            val textColor = if (isLight) Color.BLACK else Color.WHITE
            
            val btnColor = if (isLight) Color.parseColor("#E0E0E0") else Color.parseColor("#222222")
            val controlBtnColor = if (isLight) Color.parseColor("#D1D1D1") else Color.parseColor("#222222")

            findViewById<android.view.View>(R.id.main_layout)?.setBackgroundColor(bgColor)
            tvResult.setTextColor(textColor)

            val numbers = listOf(R.id.btn0, R.id.btn1, R.id.btn2, R.id.btn3, R.id.btn4, R.id.btn5, R.id.btn6, R.id.btn7, R.id.btn8, R.id.btn9)
            numbers.forEach { id ->
                findViewById<Button>(id)?.apply {
                    setBackgroundColor(btnColor)
                    setTextColor(textColor)
                }
            }

            val accentButtons = listOf(
                R.id.btnPlus, R.id.btnMinus, R.id.btnMultiply, R.id.btnDivide, 
                R.id.btnTheme, R.id.btnHistory, R.id.btnClear, R.id.btnBackspace, R.id.btnDot
            )
            accentButtons.forEach { id ->
                findViewById<Button>(id)?.apply {
                    setTextColor(accentColor)
                    setBackgroundColor(controlBtnColor)
                }
            }

            findViewById<Button>(R.id.btnEquals)?.apply {
                setBackgroundColor(accentColor)
                setTextColor(Color.WHITE)
            }

        } catch (e: Exception) {}
    }

    private fun saveActionToCloud(fullExpression: String) {
        db.collection("history").add(hashMapOf("full_expression" to fullExpression, "timestamp" to System.currentTimeMillis()))
    }

    private fun showHistoryDialog() {
        val dialog = BottomSheetDialog(this)
        val view = layoutInflater.inflate(android.R.layout.simple_list_item_1, null)
        val tv = view.findViewById<TextView>(android.R.id.text1)
        tv.setPadding(40, 40, 40, 40)
        
        db.collection("history").orderBy("timestamp", Query.Direction.DESCENDING).limit(10).get().addOnSuccessListener { docs ->
            val history = docs.joinToString("\n\n") { it.getString("full_expression") ?: "" }
            tv.text = if (history.isEmpty()) "No history yet" else "Recent History:\n\n$history"
        }
        dialog.setContentView(view)
        dialog.show()
    }

    private fun setupButtons() {
        val numbers = listOf(R.id.btn0, R.id.btn1, R.id.btn2, R.id.btn3, R.id.btn4, R.id.btn5, R.id.btn6, R.id.btn7, R.id.btn8, R.id.btn9, R.id.btnDot)
        numbers.forEach { id -> 
            val btn = findViewById<Button>(id)
            btn.setOnClickListener { tvResult.text = calculator.appendDigit(btn.text.toString(), tvResult.text.toString()) }
        }

        val ops = mapOf(R.id.btnPlus to "+", R.id.btnMinus to "-", R.id.btnMultiply to "*", R.id.btnDivide to "/")
        ops.forEach { (id, op) -> findViewById<Button>(id).setOnClickListener { calculator.setOperation(op, tvResult.text.toString()) } }

        findViewById<Button>(R.id.btnEquals).setOnClickListener {
            val second = tvResult.text.toString()
            val fullExpr = calculator.getFullExpressionForCloud(second)
            val result = calculator.calculate(second)
            tvResult.text = result
            saveActionToCloud("$fullExpr = $result")
        }

        findViewById<Button>(R.id.btnClear).setOnClickListener { tvResult.text = calculator.clear() }
        findViewById<Button>(R.id.btnBackspace).setOnClickListener { tvResult.text = calculator.backspace(tvResult.text.toString()) }
        findViewById<Button>(R.id.btnHistory).setOnClickListener { showHistoryDialog() }
        findViewById<Button>(R.id.btnTheme).setOnClickListener { toggleTheme() }
    }

    private fun setupGestures() {
        gestureDetector = GestureDetector(this, object : GestureDetector.SimpleOnGestureListener() {
            override fun onFling(e1: MotionEvent?, e2: MotionEvent, vx: Float, vy: Float): Boolean {
                if (e1 != null && (e2.x - e1.x) > 100 && abs(vx) > 100) {
                    onSwipeRight()
                    return true
                }
                return false
            }
        })
    }

    private fun onSwipeRight() {
        Toast.makeText(this, "Detecting location...", Toast.LENGTH_SHORT).show()
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.ACCESS_FINE_LOCATION), 1001)
            return
        }
        fusedLocationClient.lastLocation.addOnSuccessListener { loc ->
            if (loc != null) {
                val geocoder = Geocoder(this, Locale.getDefault())
                try {
                    val addresses = geocoder.getFromLocation(loc.latitude, loc.longitude, 1)
                    val info = if (addresses != null && addresses.isNotEmpty()) "${addresses[0].locality}, ${addresses[0].countryName}" else "${loc.latitude}, ${loc.longitude}"
                    tvResult.text = info
                    Toast.makeText(this, "Location: $info", Toast.LENGTH_LONG).show()
                } catch (e: Exception) { Toast.makeText(this, "Service unavailable", Toast.LENGTH_SHORT).show() }
            } else { Toast.makeText(this, "Turn on GPS", Toast.LENGTH_SHORT).show() }
        }
    }
}
