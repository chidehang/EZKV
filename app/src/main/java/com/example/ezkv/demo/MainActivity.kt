package com.example.ezkv.demo

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import com.cdh.ezkv.EZKV

class MainActivity : AppCompatActivity(), View.OnClickListener {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        findViewById<TextView>(R.id.tv_root_dir).text = DemoApp.rootDir
        findViewById<Button>(R.id.btn_write_int).setOnClickListener(this)
        findViewById<Button>(R.id.btn_read_int).setOnClickListener(this)
        findViewById<Button>(R.id.btn_write_string).setOnClickListener(this)
        findViewById<Button>(R.id.btn_read_string).setOnClickListener(this)

        EZKV.defaultEZKV()
    }

    override fun onClick(v: View) {
        when (v.id) {
            R.id.btn_write_int -> {
                EZKV.defaultEZKV()?.encode("kInt", -5)
            }
            R.id.btn_read_int -> {
                val value = EZKV.defaultEZKV()?.decodeInt("kInt", -1)
                Toast.makeText(this, "kInt:"+value, Toast.LENGTH_LONG).show()
            }
            R.id.btn_write_string -> {
                EZKV.defaultEZKV()?.encode("kString", "hello EZKV")
            }
            R.id.btn_read_string -> {
                val value = EZKV.defaultEZKV()?.decodeString("kString", "default string")
                Toast.makeText(this, "kString:$value", Toast.LENGTH_LONG).show()
            }
        }
    }
}