package com.example.lab.data

class Calculator {
    private var operand = 0.0
    private var operation: String? = null
    private var newInput = true

    fun appendDigit(digit: String, currentText: String): String {
        if (digit == ".") {
            if (newInput) {
                newInput = false
                return "0."
            }
            if (currentText.contains(".")) {
                return currentText
            }
        }

        val result = if (newInput || currentText == "0") {
            digit
        } else {
            currentText + digit
        }
        newInput = false
        return result
    }

    fun backspace(currentText: String): String {
        if (newInput || currentText == "Error") return "0"
        
        val result = if (currentText.length <= 1) {
            "0"
        } else {
            currentText.substring(0, currentText.length - 1)
        }
        
        if (result == "0") newInput = true
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
