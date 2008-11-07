all: checkvars bin/stasis/jni/libstasisjni.so
	@echo; echo "Be sure to add this to java's command line:" ;echo
	@echo "  -Djava.library.path=`pwd`/bin/stasis/jni" ; echo

bin/stasis/jni/Stasis.class : src/stasis/jni/Stasis.java
	cd src;	javac -d ../bin \
	    stasis/jni/Stasis.java

src/stasis/jni/stasis_jni_Stasis.h : bin/stasis/jni/Stasis.class
	javah -classpath bin -jni \
	    -d src/stasis/jni/ \
	    stasis.jni.Stasis

bin/stasis/jni/libstasisjni.so: src/stasis/jni/stasis_jni_Stasis.c \
				src/stasis/jni/stasis_jni_Stasis.h
	if [ "$(shell uname)" = "Linux" ] ; then \
	  gcc -shared -fPIC \
	      -I $(JAVA_DIR)/include \
	      -I $(STASIS_DIR) \
	      -L $(STASIS_DIR)/build/src/stasis \
	         src/stasis/jni/stasis_jni_Stasis.c \
	      -l stasis \
	      -o bin/stasis/jni/libstasisjni.so ; \
	else \
	  gcc -fPIC \
	      -I $(JAVA_DIR)/include \
	      -I $(STASIS_DIR) \
	       $(STASIS_DIR)/build/src/stasis/libstasis.dylib \
	       src/stasis/jni/stasis_jni_Stasis.c \
	       -dylib -o bin/stasis/jni/libstasisjni.dylib ; \
	fi

.PHONY: clean

clean : shutdown
	rm -rf bin/stasis/jni/Stasis.class \
	      src/stasis/jni/stasis_jni_Stasis.h \
	      bin/stasis/jni/libstasisjni.so \
	      apache-tomcat-6.0.18

.PHONY: checkvars
checkvars :
	@if [[ ! -d "$(STASIS_DIR)" ]] ; then  echo 'Define $$STASIS_DIR to continue!'; exit 1; fi
	@if [[ ! -d "$(JAVA_DIR)" ]] ; then  echo 'Define $$JAVA_DIR to continue!'; exit 1; fi


.PHONY: test
test : checkvars all
	LD_LIBRARY_PATH=$(STASIS_DIR)/build/src/stasis:$(STASIS_DIR)/build/src/libdfa \
	java -cp bin -Djava.library.path=/home/sears/workspace/jol/bin/stasis/jni stasis.jni.Stasis

apache-tomcat-6.0.18.tar.gz: 
	@wget -v http://apache.mirror99.com/tomcat/tomcat-6/v6.0.18/bin/apache-tomcat-6.0.18.tar.gz -O apache-tomcat-6.0.18.tar.gz

apache-tomcat-6.0.18: apache-tomcat-6.0.18.tar.gz
	@rm -rf apache-tomcat-6.0.18
	@echo "Verifying md5sum"
	@echo '8354e156f097158f8d7b699078fd39c1 *apache-tomcat-6.0.18.tar.gz' | md5sum -c - || exit 1;
	@tar zxf apache-tomcat-6.0.18.tar.gz

.PHONY: prepare-webapp-dir
prepare-webapp-dir: apache-tomcat-6.0.18
	@apache-tomcat-6.0.18/bin/shutdown.sh || true
	rm -rf apache-tomcat-6.0.18/webapps
	mkdir apache-tomcat-6.0.18/webapps
	@echo "Scrubbed webapp dir"

.PHONY: scrub-web-apps deploy
deploy : checkvars all prepare-webapp-dir
	ant war -q -e
	cp ant-dist/lincoln.war apache-tomcat-6.0.18/webapps
	LD_LIBRARY_PATH=$(STASIS_DIR)/build/src/stasis/ \
	CATALINA_OPTS=-Djava.library.path=bin/stasis/jni \
	apache-tomcat-6.0.18/bin/startup.sh
	@echo "Deployed"

shutdown:
	@apache-tomcat-6.0.18/bin/shutdown.sh || true

startup: checkvars
	LD_LIBRARY_PATH=$(STASIS_DIR)/build/src/stasis/ \
	CATALINA_OPTS=-Djava.library.path=bin/stasis/jni \
	apache-tomcat-6.0.18/bin/startup.sh
