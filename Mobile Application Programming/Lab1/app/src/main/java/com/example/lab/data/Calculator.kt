package com.example.lab.data

class Calculator {
    private var operand = 0.0
    private var operation: String? = null
    private var newInput = true
    private var lastFullExpression = ""

    fun appendDigit(digit: String, currentText: String): String {
        val res = if (newInput || currentText == "0" || currentText == "Error") {
            if (digit == ".") "0." else digit
        } else {
            if (digit == "." && currentText.contains(".")) currentText else currentText + digit
        }
        newInput = false
        return res
    }

    fun setOperation(op: String, currentText: String) {
        operand = currentText.toDoubleOrNull() ?: 0.0
        operation = op
        newInput = true
    }

    fun getFullExpressionForCloud(secondOperandText: String): String {
        val op = operation ?: return secondOperandText
        val first = if (operand % 1.0 == 0.0) operand.toInt().toString() else operand.toString()
        return "$first $op $secondOperandText"
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
        
        val res = if (resultValue.isNaN()) {
            "Error"
        } else {
            val formatted = resultValue.toString()
            if (formatted.endsWith(".0")) formatted.substring(0, formatted.length - 2) else formatted
        }
        
        newInput = true
        operation = null // Сбрасываем операцию после вычисления
        return res
    }

    fun clear(): String {
        operand = 0.0
        operation = null
        newInput = true
        return "0"
    }

    fun backspace(currentText: String): String {
        if (newInput || currentText == "Error" || currentText.length <= 1) return "0"
        return currentText.substring(0, currentText.length - 1)
    }
}
