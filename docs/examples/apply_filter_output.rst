.. apply_filter_output:

===================
apply_filter output
===================

This example shows how to extract the output of a filter from the :py:meth:`pmeshlab.MeshSet.apply_filter` function.
The output of this method is a python dictionary, often empty because most of the filters do not need to return anything.

When the output dictionary is not empty, it is always on the form `[key - value]`, where the `key` is of `str` type and the `value` can be an `int`, `float` or a numpy array depending on what is best to represent the returned value.
Right now the :ref:`filter_list` documentation lacks on if and what type of values a filter returns (we are sorry about that!). We will expand the documentation in the future; in the meantime we suggest to look for the name and type of values returned by running the desired filter from a python console.

This script can be executed by running the following command:

.. code-block:: none

   pytest --pyargs pymeshlab -k 'compute_geometric_measures'

**tests/test_compute_geometric_measures.py**

.. include:: ../../pymeshlab/tests/test_compute_geometric_measures.py
   :literal:
