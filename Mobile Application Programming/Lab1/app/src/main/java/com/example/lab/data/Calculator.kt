package com.example.lab.data

class Calculator {
    private var operand = 0.0
    private var operation: String? = null
    private var newInput = true

    fun appendDigit(digit: String, currentText: String): String {
        val result = if (newInput || currentText == "0") {
            digit
        } else {
            currentText + digit
        }
        newInput = false
        return result
    }

    fun setOperation(op: String, currentText: String) {
        operand = currentText.toDoubleOrNull() ?: 0.0
        operation = op
        newInput = true
    }

    fun calculate(currentText: String): String {
        val second = currentText.toDoubleOrNull() ?: 0.0
        val resultValue = when (operation) {
            "+" -> operand + second
            "-" -> operand - second
            "*" -> operand * second
            "/" -> if (second != 0.0) operand / second else Double.NaN
            else -> second
        }
        newInput = true
        
        return if (resultValue.isNaN()) {
            "Error"
        } else {
            // Убираем .0 если число целое
            val formatted = resultValue.toString()
            if (formatted.endsWith(".0")) {
                formatted.substring(0, formatted.length - 2)
            } else {
                formatted
            }
        }
    }

    fun clear(): String {
        operand = 0.0
        operation = null
        newInput = true
        return "0"
    }
}
