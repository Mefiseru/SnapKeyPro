#include "strafe_tapper.h"

#ifdef _WIN32

#include <unordered_map>

struct KeyState {
    bool pressed = false;
};

// Key state management
std::unordered_map<int, KeyState> keyStates;

// Active and previous keys for SnapKey logic
int activeKey = 0;
int previousKey = 0;

// Function to check if a key is currently pressed
PyObject* check_key_state(PyObject* self, PyObject* args) {
    char* key;
    if (!PyArg_ParseTuple(args, "s", &key))
        return NULL;

    SHORT keyCode;
    if (strcmp(key, "A") == 0) {
        keyCode = 0x41; // Virtual key code for 'A'
    } else if (strcmp(key, "D") == 0) {
        keyCode = 0x44; // Virtual key code for 'D'
    } else if (strcmp(key, "N") == 0) {
        keyCode = 0x4E; // Virtual key code for 'N'
    } else {
        Py_RETURN_FALSE;
    }

    // Return True if the key is pressed
    if (GetAsyncKeyState(keyCode) & 0x8000) {
        Py_RETURN_TRUE;  // Key is pressed
    } else {
        Py_RETURN_FALSE;  // Key is released
    }
}

// Function to simulate a key press (no delay)
void press_key(SHORT keyCode) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;

    // Press the key
    if (!SendInput(1, &input, sizeof(INPUT))) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to send key press input");
        return;
    }
}

// Function to simulate a key release
void release_key(SHORT keyCode) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = KEYEVENTF_KEYUP;

    // Release the key
    if (!SendInput(1, &input, sizeof(INPUT))) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to send key release input");
        return;
    }
}

// Function to simulate a key tap with a specified hold duration
PyObject* tap_key(PyObject* self, PyObject* args) {
    char* key;
    int hold_time;
    if (!PyArg_ParseTuple(args, "si", &key, &hold_time))
        return NULL;

    SHORT keyCode;
    if (strcmp(key, "A") == 0) {
        keyCode = 0x41; // Virtual key code for 'A'
    } else if (strcmp(key, "D") == 0) {
        keyCode = 0x44; // Virtual key code for 'D'
    } else {
        Py_RETURN_FALSE;
    }

    // Press the key immediately
    press_key(keyCode);

    // Hold the key for specified milliseconds (delay for counter-tap)
    Sleep(hold_time);

    // Release the key
    release_key(keyCode);

    Py_RETURN_NONE;  // Return None to indicate success
}

// Function to simulate a key hold
PyObject* hold_key(PyObject* self, PyObject* args) {
    char* key;
    if (!PyArg_ParseTuple(args, "s", &key))
        return NULL;

    SHORT keyCode;
    if (strcmp(key, "A") == 0) {
        keyCode = 0x41; // Virtual key code for 'A'
    } else if (strcmp(key, "D") == 0) {
        keyCode = 0x44; // Virtual key code for 'D'
    } else {
        Py_RETURN_FALSE;
    }

    // Only press the key if it is not already pressed
    if (!keyStates[keyCode].pressed) {
        keyStates[keyCode].pressed = true;
        press_key(keyCode);
    }

    Py_RETURN_NONE;  // Return None to indicate success
}

// Function to release a held key
PyObject* release_held_key(PyObject* self, PyObject* args) {
    char* key;
    if (!PyArg_ParseTuple(args, "s", &key))
        return NULL;

    SHORT keyCode;
    if (strcmp(key, "A") == 0) {
        keyCode = 0x41; // Virtual key code for 'A'
    } else if (strcmp(key, "D") == 0) {
        keyCode = 0x44; // Virtual key code for 'D'
    } else {
        Py_RETURN_FALSE;
    }

    // Only release the key if it is currently pressed
    if (keyStates[keyCode].pressed) {
        keyStates[keyCode].pressed = false;
        release_key(keyCode);

        if (activeKey == keyCode) {
            activeKey = 0;
        }

        if (previousKey == keyCode) {
            previousKey = 0;
        }
    }

    Py_RETURN_NONE;  // Return None to indicate success
}

// Function to implement SnapKey feature
PyObject* snap_key(PyObject* self, PyObject* args) {
    char* held_key;
    char* new_key;
    int hold_time;
    if (!PyArg_ParseTuple(args, "ssi", &held_key, &new_key, &hold_time))
        return NULL;

    SHORT heldKeyCode, newKeyCode;
    if (strcmp(held_key, "A") == 0) {
        heldKeyCode = 0x41; // Virtual key code for 'A'
    } else if (strcmp(held_key, "D") == 0) {
        heldKeyCode = 0x44; // Virtual key code for 'D'
    } else {
        Py_RETURN_FALSE;
    }

    if (strcmp(new_key, "A") == 0) {
        newKeyCode = 0x41;
    } else if (strcmp(new_key, "D") == 0) {
        newKeyCode = 0x44;
    } else {
        Py_RETURN_FALSE;
    }

    // Get the current state of keys
    auto& heldKeyState = keyStates[heldKeyCode];
    auto& newKeyState = keyStates[newKeyCode];

    // Switch to the new key if it's not the active key
    if (!newKeyState.pressed) {
        newKeyState.pressed = true;
        previousKey = activeKey;
        activeKey = newKeyCode;

        // Release the previous key if needed
        if (previousKey != 0 && previousKey != activeKey) {
            release_key(previousKey);
        }

        // Press the new key
        press_key(activeKey);
    }

    // Handle key release logic
    if (heldKeyState.pressed && !(GetAsyncKeyState(heldKeyCode) & 0x8000)) {
        heldKeyState.pressed = false;

        if (activeKey == newKeyCode && previousKey != 0) {
            // Revert to the previous key
            release_key(activeKey);
            activeKey = previousKey;
            previousKey = 0;
            press_key(activeKey);
        }
    }

    Py_RETURN_NONE;  // Return None to indicate success
}

#elif __linux__

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <unordered_map>

static Display* display = XOpenDisplay(NULL);
static Window root = DefaultRootWindow(display);

struct KeyState {
    bool pressed = false;
};

// Key state management
std::unordered_map<int, KeyState> keyStates;

// Active and previous keys for SnapKey logic
int activeKey = 0;
int previousKey = 0;

// Function to check if a key is currently pressed
PyObject* check_key_state(PyObject* self, PyObject* args) {
    char* key;
    if (!PyArg_ParseTuple(args, "s", &key))
        return NULL;

    char keys_return[32];
    XQueryKeymap(display, keys_return);

    KeyCode keycode;
    if (strcmp(key, "A") == 0) {
        keycode = XKeysymToKeycode(display, XK_A);
    } else if (strcmp(key, "D") == 0) {
        keycode = XKeysymToKeycode(display, XK_D);
    } else if (strcmp(key, "N") == 0) {
        keycode = XKeysymToKeycode(display, XK_N);
    } else {
        Py_RETURN_FALSE;
    }

    // Return True if the key is pressed
    if (keys_return[keycode / 8] & (1 << (keycode % 8))) {
        Py_RETURN_TRUE;  // Key is pressed
    } else {
        Py_RETURN_FALSE;  // Key is released
    }
}

// Function to simulate a key press (no delay)
void press_key_linux(KeyCode keycode) {
    XTestFakeKeyEvent(display, keycode, True, CurrentTime);
    XFlush(display);
}

// Function to simulate a key release
void release_key_linux(KeyCode keycode) {
    XTestFakeKeyEvent(display, keycode, False, CurrentTime);
    XFlush(display);
}

// Function to simulate a key tap with a specified hold duration
PyObject* tap_key(PyObject* self, PyObject* args) {
    char* key;
    int hold_time;
    if (!PyArg_ParseTuple(args, "si", &key, &hold_time))
        return NULL;

    KeySym keysym;
    KeyCode keycode;
    if (strcmp(key, "A") == 0) {
        keysym = XK_A;
        keycode = XKeysymToKeycode(display, XK_A);
    } else if (strcmp(key, "D") == 0) {
        keysym = XK_D;
        keycode = XKeysymToKeycode(display, XK_D);
    } else {
        Py_RETURN_FALSE;
    }

    // Press the key immediately
    press_key_linux(keycode);

    // Hold the key for specified milliseconds (delay for counter-tap)
    usleep(hold_time * 1000);  // Convert to microseconds

    // Release the key
    release_key_linux(keycode);

    Py_RETURN_NONE;  // Return None to indicate success
}

// Function to simulate a key hold
PyObject* hold_key(PyObject* self, PyObject* args) {
    char* key;
    if (!PyArg_ParseTuple(args, "s", &key))
        return NULL;

    KeyCode keycode;
    if (strcmp(key, "A") == 0) {
        keycode = XKeysymToKeycode(display, XK_A);
    } else if (strcmp(key, "D") == 0) {
        keycode = XKeysymToKeycode(display, XK_D);
    } else {
        Py_RETURN_FALSE;
    }

    // Only press the key if it is not already pressed
    if (!keyStates[keycode].pressed) {
        keyStates[keycode].pressed = true;
        press_key_linux(keycode);
    }

    Py_RETURN_NONE;  // Return None to indicate success
}

// Function to release a held key
PyObject* release_held_key(PyObject* self, PyObject* args) {
    char* key;
    if (!PyArg_ParseTuple(args, "s", &key))
        return NULL;

    KeyCode keycode;
    if (strcmp(key, "A") == 0) {
        keycode = XKeysymToKeycode(display, XK_A);
    } else if (strcmp(key, "D") == 0) {
        keycode = XKeysymToKeycode(display, XK_D);
    } else {
        Py_RETURN_FALSE;
    }

    // Only release the key if it is currently pressed
    if (keyStates[keycode].pressed) {
        keyStates[keycode].pressed = false;
        release_key_linux(keycode);

        if (activeKey == keycode) {
            activeKey = 0;
        }

        if (previousKey == keycode) {
            previousKey = 0;
        }
    }

    Py_RETURN_NONE;  // Return None to indicate success
}

// Function to implement SnapKey feature
PyObject* snap_key(PyObject* self, PyObject* args) {
    char* held_key;
    char* new_key;
    int hold_time;
    if (!PyArg_ParseTuple(args, "ssi", &held_key, &new_key, &hold_time))
        return NULL;

    KeySym held_keysym, new_keysym;
    KeyCode held_keycode, new_keycode;

    if (strcmp(held_key, "A") == 0) {
        held_keysym = XK_A;
        held_keycode = XKeysymToKeycode(display, XK_A);
    } else if (strcmp(held_key, "D") == 0) {
        held_keysym = XK_D;
        held_keycode = XKeysymToKeycode(display, XK_D);
    } else {
        Py_RETURN_FALSE;
    }

    if (strcmp(new_key, "A") == 0) {
        new_keysym = XK_A;
        new_keycode = XKeysymToKeycode(display, XK_A);
    } else if (strcmp(new_key, "D") == 0) {
        new_keysym = XK_D;
        new_keycode = XKeysymToKeycode(display, XK_D);
    } else {
        Py_RETURN_FALSE;
    }

    // Get the current state of keys
    auto& heldKeyState = keyStates[held_keycode];
    auto& newKeyState = keyStates[new_keycode];

    // Switch to the new key if it's not the active key
    if (!newKeyState.pressed) {
        newKeyState.pressed = true;
        previousKey = activeKey;
        activeKey = new_keycode;

        // Release the previous key if needed
        if (previousKey != 0 && previousKey != activeKey) {
            release_key_linux(previousKey);
        }

        // Press the new key
        press_key_linux(activeKey);
    }

    // Handle key release logic
    char keys_return[32];
    XQueryKeymap(display, keys_return);

    if (heldKeyState.pressed && !(keys_return[held_keycode / 8] & (1 << (held_keycode % 8)))) {
        heldKeyState.pressed = false;

        if (activeKey == new_keycode && previousKey != 0) {
            // Revert to the previous key
            release_key_linux(activeKey);
            activeKey = previousKey;
            previousKey = 0;
            press_key_linux(activeKey);
        }
    }

    Py_RETURN_NONE;  // Return None to indicate success
}

#endif

static PyMethodDef StrafeTapperMethods[] = {
    {"check_key_state", check_key_state, METH_VARARGS, "Check if a key is currently pressed."},
    {"tap_key", tap_key, METH_VARARGS, "Tap a key with hold duration."},
    {"hold_key", hold_key, METH_VARARGS, "Hold a key down without releasing."},
    {"release_held_key", release_held_key, METH_VARARGS, "Release a held key."},
    {"snap_key", snap_key, METH_VARARGS, "SnapKey feature implementation."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef strafetappermodule = {
    PyModuleDef_HEAD_INIT,
    "strafe_tapper",
    NULL,
    -1,
    StrafeTapperMethods
};

PyMODINIT_FUNC PyInit_strafe_tapper(void) {
#ifdef __linux__
    if (display == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Unable to open X display");
        return NULL;
    }
#endif

    return PyModule_Create(&strafetappermodule);
}

// Cleanup function to close the display
void cleanup() {
#ifdef __linux__
    if (display != NULL) {
        XCloseDisplay(display);
    }
#endif
}
