all: bin/stasis/jni/libstasisjni.so
	@echo; echo "Be sure to add this to java's command line:" ;echo
	@echo "  -Djava.library.path=`pwd`/bin/stasis/jni" ; echo

STASIS_DIR=/home/sears/stasis4

bin/stasis/jni/Stasis.class : src/stasis/jni/Stasis.java
	cd src;	javac -d ../bin \
	    stasis/jni/Stasis.java

src/stasis/jni/stasis_jni_Stasis.h : bin/stasis/jni/Stasis.class
	javah -classpath bin -jni \
	    -d src/stasis/jni/ \
	    stasis.jni.Stasis

bin/stasis/jni/libstasisjni.so: src/stasis/jni/stasis_jni_Stasis.c \
				src/stasis/jni/stasis_jni_Stasis.h
	gcc -shared -fPIC \
	    -I /usr/lib/jvm/java-6-openjdk/include \
            -I $(STASIS_DIR) \
	    -L $(STASIS_DIR)/build/src/libdfa \
	    -L $(STASIS_DIR)/build/src/stasis \
               src/stasis/jni/stasis_jni_Stasis.c \
	    -l stasis \
            -l rw \
	    -o bin/stasis/jni/libstasisjni.so

.PHONY: clean

clean :
	rm bin/stasis/jni/Stasis.class \
	   src/stasis/jni/stasis_jni_Stasis.h \
	   bin/stasis/jni/libstasisjni.so

.PHONY: test
test : all
	java -cp bin -Djava.library.path=/home/sears/workspace/jol/bin/stasis/jni stasis.jni.Stasis