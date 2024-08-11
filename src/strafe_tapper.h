#ifndef STRAFE_TAPPER_H
#define STRAFE_TAPPER_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <Python.h>

// Function declarations
PyObject* check_key_state(PyObject* self, PyObject* args);
PyObject* tap_key(PyObject* self, PyObject* args);
PyObject* hold_key(PyObject* self, PyObject* args);
PyObject* release_held_key(PyObject* self, PyObject* args);
PyObject* snap_key(PyObject* self, PyObject* args);

#endif // STRAFE_TAPPER_H
