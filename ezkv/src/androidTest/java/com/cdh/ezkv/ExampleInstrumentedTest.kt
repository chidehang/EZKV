package com.cdh.ezkv

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.assertEquals
import org.junit.BeforeClass
import org.junit.FixMethodOrder
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.MethodSorters

/**
 * Instrumented test, which will execute on an Android device.
 *
 * See [testing documentation](http://d.android.com/tools/testing).
 */
@RunWith(AndroidJUnit4::class)
@FixMethodOrder(value = MethodSorters.NAME_ASCENDING)
class ExampleInstrumentedTest {

    companion object {

        lateinit var kv: EZKV

        @BeforeClass
        @JvmStatic
        fun initial() {
            val context = InstrumentationRegistry.getInstrumentation().targetContext
            EZKV.initialize(context)
            kv = EZKV.defaultEZKV()
        }

    }

    @Test
    fun test01GrowFile() {
        for (i in 0 .. 200) {
            kv.encode("bigEntry$i", "abcdefgh")
        }
        assertEquals(kv.totalSize(), EZKV.pageSize() * 2)
        assertEquals(kv.count(), 201)
    }
    
    @Test
    fun test02Int() {
        assertEquals(kv.encode("int", Integer.MAX_VALUE), true)
        assertEquals(kv.decodeInt("int", -1), Integer.MAX_VALUE)
    }

    @Test
    fun test03String() {
        assertEquals(kv.encode("str", "haha ezkv"), true)
        assertEquals(kv.decodeString("str", "default"), "haha ezkv")
    }

    @Test
    fun test04RemoveKey() {
        assertEquals(kv.encode("removeKey", "abc"), true)
        kv.removeValueForKey("removeKey")
        assertEquals(kv.decodeString("removeKey", "default"), "default")
    }
}