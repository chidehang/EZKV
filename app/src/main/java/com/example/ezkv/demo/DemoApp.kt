package com.example.ezkv.demo

import android.app.Application
import com.cdh.ezkv.EZKV

/**
 * Created by chidehang on 2021/3/24
 */
class DemoApp : Application() {

    override fun onCreate() {
        super.onCreate()
        rootDir = EZKV.initialize(this)
    }

    companion object {
        var rootDir:String ?= ""
    }
}