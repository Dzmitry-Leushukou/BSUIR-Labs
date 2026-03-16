package com.example.lab.ui

import android.Manifest
import android.content.pm.PackageManager
import android.graphics.Color
import android.location.Geocoder
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.GestureDetector
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.core.view.WindowInsetsControllerCompat
import com.example.lab.R
import com.example.lab.data.Calculator
import com.example.lab.data.SecurityManager
import com.google.android.gms.location.FusedLocationProviderClient
import com.google.android.gms.location.LocationServices
import com.google.android.material.bottomsheet.BottomSheetDialog
import com.google.firebase.firestore.FirebaseFirestore
import com.google.firebase.firestore.Query
import com.google.firebase.messaging.FirebaseMessaging
import com.google.firebase.remoteconfig.FirebaseRemoteConfig
import com.google.firebase.remoteconfig.FirebaseRemoteConfigSettings
import java.util.Locale
import kotlin.math.abs

class MainActivity : AppCompatActivity() {

    private lateinit var calculator: Calculator
    private lateinit var securityManager: SecurityManager
    private lateinit var tvResult: TextView
    private lateinit var gestureDetector: GestureDetector
    private lateinit var fusedLocationClient: FusedLocationProviderClient
    
    private val db = FirebaseFirestore.getInstance()
    private val remoteConfig = FirebaseRemoteConfig.getInstance()
    
    private var cloudAccentColor = "#43A047"
    private var isLightTheme = false
    private var isAuthenticated = false

    override fun onCreate(savedInstanceState: Bundle?) {
        installSplashScreen()
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        calculator = Calculator()
        securityManager = SecurityManager(this)
        tvResult = findViewById(R.id.tvResult)
        fusedLocationClient = LocationServices.getFusedLocationProviderClient(this)

        setupButtons()
        setupGestures()
        setupRemoteConfig()
        loadSavedTheme()
        setupPushNotifications()
        
        // Step 4: Запуск авторизации
        startAuthorizationFlow()

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.POST_NOTIFICATIONS), 1002)
        }
    }

    private fun startAuthorizationFlow() {
        if (!securityManager.isPassKeySet()) {
            // Критерий 1: Инициализация Pass Key при первом запуске
            showPasskeyDialog(isSetup = true)
        } else {
            // Критерий 3: Проверка Pass Key или биометрии
            if (securityManager.canAuthenticate()) {
                securityManager.showBiometricPrompt(this, 
                    onSuccess = { unlockApp() },
                    onError = { showPasskeyDialog(isSetup = false) }
                )
            } else {
                showPasskeyDialog(isSetup = false)
            }
        }
    }

    private fun showPasskeyDialog(isSetup: Boolean) {
        val dialogView = LayoutInflater.from(this).inflate(R.layout.dialog_passkey, null)
        val etPasskey = dialogView.findViewById<EditText>(R.id.etPasskey)
        val btnAuth = dialogView.findViewById<Button>(R.id.btnAuth)
        val tvTitle = dialogView.findViewById<TextView>(R.id.tvPasskeyTitle)

        tvTitle.text = if (isSetup) "Set New Pass Key (4 digits)" else "Enter Pass Key"
        btnAuth.text = if (isSetup) "Set Key" else "Authorize"

        val dialog = AlertDialog.Builder(this)
            .setView(dialogView)
            .setCancelable(false)
            .create()

        btnAuth.setOnClickListener {
            val input = etPasskey.text.toString()
            if (input.length == 4) {
                if (isSetup) {
                    securityManager.savePassKey(input)
                    unlockApp()
                    dialog.dismiss()
                } else {
                    if (securityManager.validatePassKey(input)) {
                        unlockApp()
                        dialog.dismiss()
                    } else {
                        // Критерий 2: Обработка неверного ключа
                        Toast.makeText(this, "Incorrect Pass Key!", Toast.LENGTH_SHORT).show()
                        etPasskey.text.clear()
                    }
                }
            } else {
                Toast.makeText(this, "Enter 4 digits", Toast.LENGTH_SHORT).show()
            }
        }
        dialog.show()
    }

    private fun unlockApp() {
        isAuthenticated = true
        findViewById<View>(R.id.main_layout).visibility = View.VISIBLE
        Toast.makeText(this, "Access Granted", Toast.LENGTH_SHORT).show()
    }

    private fun setupPushNotifications() {
        FirebaseMessaging.getInstance().token.addOnCompleteListener { task ->
            if (task.isSuccessful) Log.d("PushAPI", "Token: ${task.result}")
        }
    }

    override fun dispatchTouchEvent(ev: MotionEvent?): Boolean {
        if (!isAuthenticated) return false // Блокируем касания до авторизации
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
    }

    private fun setupRemoteConfig() {
        val configSettings = FirebaseRemoteConfigSettings.Builder().setMinimumFetchIntervalInSeconds(0).build()
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
            val bgColor = if (isLight) Color.parseColor("#F5F5F5") else Color.BLACK
            window.statusBarColor = bgColor
            WindowInsetsControllerCompat(window, window.decorView).isAppearanceLightStatusBars = isLight
            
            val textColor = if (isLight) Color.BLACK else Color.WHITE
            val btnColor = if (isLight) Color.parseColor("#E0E0E0") else Color.parseColor("#222222")
            val controlBtnColor = if (isLight) Color.parseColor("#D1D1D1") else Color.parseColor("#222222")

            findViewById<View>(R.id.main_layout)?.setBackgroundColor(bgColor)
            tvResult.setTextColor(textColor)

            val numbers = listOf(R.id.btn0, R.id.btn1, R.id.btn2, R.id.btn3, R.id.btn4, R.id.btn5, R.id.btn6, R.id.btn7, R.id.btn8, R.id.btn9)
            numbers.forEach { id ->
                findViewById<Button>(id)?.apply {
                    setBackgroundColor(btnColor)
                    setTextColor(textColor)
                }
            }

            val accentButtons = listOf(R.id.btnPlus, R.id.btnMinus, R.id.btnMultiply, R.id.btnDivide, R.id.btnTheme, R.id.btnHistory, R.id.btnClear, R.id.btnBackspace, R.id.btnDot)
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
            val result = calculator.calculate(second)
            tvResult.text = result
            db.collection("history").add(hashMapOf("full_expression" to "$result", "timestamp" to System.currentTimeMillis()))
        }

        findViewById<Button>(R.id.btnClear).setOnClickListener { tvResult.text = calculator.clear() }
        findViewById<Button>(R.id.btnBackspace).setOnClickListener { tvResult.text = calculator.backspace(tvResult.text.toString()) }
        findViewById<Button>(R.id.btnHistory).setOnClickListener {
            val dialog = BottomSheetDialog(this)
            val view = layoutInflater.inflate(android.R.layout.simple_list_item_1, null)
            val tv = view.findViewById<TextView>(android.R.id.text1)
            db.collection("history").orderBy("timestamp", Query.Direction.DESCENDING).limit(10).get().addOnSuccessListener { docs ->
                tv.text = docs.joinToString("\n\n") { it.getString("full_expression") ?: "" }
            }
            dialog.setContentView(view)
            dialog.show()
        }
        findViewById<Button>(R.id.btnTheme).setOnClickListener { toggleTheme() }
        
        // Критерий 4: Опция смены Pass Key (по долгому нажатию на кнопку темы, например)
        findViewById<Button>(R.id.btnTheme).setOnLongClickListener {
            showPasskeyDialog(isSetup = true)
            true
        }
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
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.ACCESS_FINE_LOCATION), 1001)
            return
        }
        fusedLocationClient.lastLocation.addOnSuccessListener { loc ->
            if (loc != null) {
                val geocoder = Geocoder(this, Locale.getDefault())
                val addresses = geocoder.getFromLocation(loc.latitude, loc.longitude, 1)
                tvResult.text = addresses?.get(0)?.locality ?: "Unknown"
            }
        }
    }
}
