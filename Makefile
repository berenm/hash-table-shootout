LINK:=-flto -fwhole-program -Wl,-O1
OPTIMIZE:=-O3
CC:=gcc
CXX:=g++

all: build/glib_hash_table build/stl_unordered_map build/stl_map build/boost_unordered_map build/google_sparse_hash_map build/google_dense_hash_map build/qt_qhash build/python_dict build/bplus-tree-v1 build/bplus-tree-v2 build/glib_tree build/bptree
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

build/bplus-tree-v1: src/bplus-tree-v1.c bplus-tree-v1/src/*.[ch] bplus-tree-v1/include/*.h Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=32 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus-tree-v1.c -o build/bplus-tree-v1 -Ibplus-tree-v1/test/ -Ibplus-tree-v1/src/ -Ibplus-tree-v1/include/

build/bplus-tree-v2: src/bplus-tree-v2.c bplus-tree-v2/src/*.[ch] bplus-tree-v2/include/*.h Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG -std=gnu99 -DBPLUS_TREE_ORDER=32 -DBNPPP_MEMORY_USE_GLIB  $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/bplus-tree-v2.c -o build/bplus-tree-v2 -Ibplus-tree-v2/test/ -Ibplus-tree-v2/src/ -Ibplus-tree-v2/include/

build/glib_tree: src/glib_tree.c Makefile src/template.c
	$(CC) -DG_DISABLE_CHECKS -DG_DISABLE_ASSERT -D_NDEBUG $(OPTIMIZE) $(LINK) -lm `pkg-config --cflags --libs glib-2.0` src/glib_tree.c -o build/glib_tree

build/bptree: src/bptree.* Makefile src/template.c
	$(CC) -std=gnu99 $(OPTIMIZE) $(LINK) -lm src/bptree.c -o build/bptree
