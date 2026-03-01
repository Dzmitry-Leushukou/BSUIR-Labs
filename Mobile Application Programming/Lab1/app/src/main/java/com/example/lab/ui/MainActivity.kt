package com.example.lab.ui

import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.example.lab.R
import com.example.lab.data.Calculator

class MainActivity : AppCompatActivity() {

    private lateinit var calculator: Calculator
    private lateinit var tvResult: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        calculator = Calculator()
        tvResult = findViewById(R.id.tvResult)

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
}
