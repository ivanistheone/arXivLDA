/* Simple example demonstrating
   how one can extend python using C 
   without using ctypes, or Boost or SWIG or anything...

*/

#include <Python.h>


static PyObject*
fib(PyObject *self, PyObject *args)
{

    //    Then we get to the body of the fucntion. First, let’s initialize some variables:
    int a = 0, b = 1, c, n;

    //    Then we move on to parsing the arguments passed to the function. For this we utilize PyArg_ParseTuple. Read more in the documentation page Parsing arguments and building values, which gives an overview on how to parse different types of arguments. Our example, however, only accepts a single integer. If that doesn’t work, we return NULL.

    if (!PyArg_ParseTuple(args, "i", &n))
            return NULL;


    //Then we instantiate a new Python list, using PyList_New, which accepts an integer as the length of the list. Since we don’t know how long the list will be when we finish, we start with zero.

    PyObject *list = PyList_New(0);


    // Then we get to the guts of the actual calculation. The line we pay attention to is the PyList_Append(list, PyInt_FromLong(b));, as that is where and how we add another item to the list. PyList_Append is analogous to Python’s list.append() method. We use PyInt_FromLong to create a Python object from the integer in the loop.

    while (b<n) {
        PyList_Append(list, PyInt_FromLong(b));
        c=a+b;
        a=b;
        b=c;
    }

    return list;    //a PyList object pointer ...

}




//That makes up the guts of the function, but how do we integrate this into Python as a module? First we create a PyMethodDef object with the functions we want to build into the module. Since we only have the one function, we only have one definition, like so:

PyMethodDef methods[] = {
    {"fib", fib, METH_VARARGS, "Returns a fibonacci sequence as a list"},
    {NULL, NULL, 0, NULL}
};

//The last step is to initialize the module. To understand what’s going on here, read through this page as it has a thorough explanation of everything happening.

PyMODINIT_FUNC 
initfib()
{
        (void) Py_InitModule("fib", methods);   
}




