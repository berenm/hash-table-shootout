LINK:=-flto -fwhole-program -Wl,-O1
OPTIMIZE:=-O3
CC:=gcc
CXX:=g++

all: build/glib_hash_table build/stl_unordered_map build/stl_map build/boost_unordered_map build/google_sparse_hash_map build/google_dense_hash_map build/qt_qhash build/python_dict build/glib_hash_table_lite build/bplus_tree build/glib_tree build/bplus_tree_dicho build/bplus_tree2 build/bplus_tree2_linear build/bplus_tree_generic build/bplus_tree_generic_linear build/bptree
# build/stx_bplus_tree 

build/glib_hash_table: src/glib_hash_table.c Makefile src/template.c
	$(CC)  -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/glib_hash_table.c -o build/glib_hash_table

build/stl_unordered_map: src/stl_unordered_map.cc Makefile src/template.c
	$(CXX)  $(OPTIMIZE) $(LINK) -lm src/stl_unordered_map.cc -o build/stl_unordered_map -std=c++0x

build/stl_map: src/stl_map.cc Makefile src/template.c
	$(CXX)  $(OPTIMIZE) $(LINK) -lm src/stl_map.cc -o build/stl_map -std=c++0x

build/stx_bplus_tree: src/stx_bplus_tree.cc Makefile src/template.c src/stx/*
	$(CXX)  $(OPTIMIZE) $(LINK) -lm src/stx_bplus_tree.cc -o build/stx_bplus_tree -std=c++0x -Isrc/

build/boost_unordered_map: src/boost_unordered_map.cc Makefile src/template.c
	$(CXX)  $(OPTIMIZE) $(LINK) -lm src/boost_unordered_map.cc -o build/boost_unordered_map

build/google_sparse_hash_map: src/google_sparse_hash_map.cc Makefile src/template.c
	$(CXX)  $(OPTIMIZE) $(LINK) -lm src/google_sparse_hash_map.cc -o build/google_sparse_hash_map

build/google_dense_hash_map: src/google_dense_hash_map.cc Makefile src/template.c
	$(CXX)  $(OPTIMIZE) $(LINK) -lm src/google_dense_hash_map.cc -o build/google_dense_hash_map

build/qt_qhash: src/qt_qhash.cc Makefile src/template.c
	$(CXX)  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs QtCore` src/qt_qhash.cc -o build/qt_qhash

build/python_dict: src/python_dict.c Makefile src/template.c
	$(CC)  $(OPTIMIZE) $(LINK) -lm -I/usr/include/python2.6 -lpython2.6 src/python_dict.c -o build/python_dict

# build/ruby_hash: src/ruby_hash.c Makefile src/template.c
# 	$(CC) $(OPTIMIZE) $(LINK) -lm -I/usr/include/ruby-1.9.1 -I /usr/include/ruby-1.9.1/x86_64-linux -lruby-1.9.1 src/ruby_hash.c -o build/ruby_hash

build/glib_hash_table_lite: src/glib_hash_table_lite.* Makefile src/template.c
	$(CC)  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/glib_hash_table_lite.c -o build/glib_hash_table_lite

build/bplus_tree: build/bplus_tree_16 build/bplus_tree_32 build/bplus_tree_64 build/bplus_tree_128 build/bplus_tree_256

build/bplus_tree_16: src/bplus_tree.* Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=16 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree.c -o build/bplus_tree_16

build/bplus_tree_32: src/bplus_tree.* Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=32 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree.c -o build/bplus_tree_32

build/bplus_tree_64: src/bplus_tree.* Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=64 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree.c -o build/bplus_tree_64

build/bplus_tree_128: src/bplus_tree.* Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=128 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree.c -o build/bplus_tree_128

build/bplus_tree_256: src/bplus_tree.* Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=256 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree.c -o build/bplus_tree_256

build/bplus_tree_dicho: build/bplus_tree_dicho_16 build/bplus_tree_dicho_32 build/bplus_tree_dicho_64 build/bplus_tree_dicho_128 build/bplus_tree_dicho_256

build/bplus_tree_dicho_16: src/bplus_tree_dicho.* Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=16 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree_dicho.c -o build/bplus_tree_dicho_16

build/bplus_tree_dicho_32: src/bplus_tree_dicho.* Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=32 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree_dicho.c -o build/bplus_tree_dicho_32

build/bplus_tree_dicho_64: src/bplus_tree_dicho.* Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=64 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree_dicho.c -o build/bplus_tree_dicho_64

build/bplus_tree_dicho_128: src/bplus_tree_dicho.* Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=128 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree_dicho.c -o build/bplus_tree_dicho_128

build/bplus_tree_dicho_256: src/bplus_tree_dicho.* Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=256 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree_dicho.c -o build/bplus_tree_dicho_256

build/glib_tree: src/glib_tree.c Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/glib_tree.c -o build/glib_tree

build/bplus_tree2: src/bplus_tree2.* bplus_tree/src/*.[ch] bplus_tree/include/*.h Makefile src/template.c
	$(CC) -std=gnu99 -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree2.c -o build/bplus_tree2 -Ibplus_tree/src -Ibplus_tree/include

build/bplus_tree2_linear: src/bplus_tree2.* bplus_tree/src/*.[ch] bplus_tree/include/*.h Makefile src/template.c
	$(CC) -std=gnu99 -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -DBPLUS_TREE_LINEAR_SEARCH $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree2.c -o build/bplus_tree2_linear -Ibplus_tree/src -Ibplus_tree/include

build/bplus_tree_generic: src/bplus_tree2.* bplus_tree/src/*.[ch] bplus_tree/include/*.h Makefile src/template.c
	$(CC) -std=gnu99 -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -DBPLUS_TREE_GENERIC $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree2.c -o build/bplus_tree_generic -Ibplus_tree/src -Ibplus_tree/include

build/bplus_tree_generic_linear: src/bplus_tree2.* bplus_tree/src/*.[ch] bplus_tree/include/*.h Makefile src/template.c
	$(CC) -std=gnu99 -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -DBPLUS_TREE_LINEAR_SEARCH -DBPLUS_TREE_GENERIC $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus_tree2.c -o build/bplus_tree_generic_linear -Ibplus_tree/src -Ibplus_tree/include

build/bptree: src/bptree.* Makefile src/template.c
	$(CC) -std=gnu99 $(OPTIMIZE) $(LINK) -lm src/bptree.c -o build/bptree
