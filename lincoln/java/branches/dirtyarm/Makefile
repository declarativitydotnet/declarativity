all: checkvars ant-build/stasis/jni/libstasisjni.so
	@echo; echo "Be sure to add this to java's command line:" ;echo
	@echo "  -Djava.library.path=`pwd`/ant-build/stasis/jni" ; echo

ant-build/stasis/jni/Stasis.class : src/stasis/jni/Stasis.java
	mkdir ant-build
	cd src;	javac -d ../ant-build \
	    stasis/jni/Stasis.java

src/stasis/jni/stasis_jni_Stasis.h : ant-build/stasis/jni/Stasis.class
	javah -classpath ant-build -jni \
	    -d src/stasis/jni/ \
	    stasis.jni.Stasis

ant-build/stasis/jni/libstasisjni.so: src/stasis/jni/stasis_jni_Stasis.c \
				src/stasis/jni/stasis_jni_Stasis.h
	## XXX include/linux  worksaround broken sun java ubuntu package
	if [ "$(shell uname)" = "Linux" ] ; then \
	  gcc -shared -fPIC \
	      -I $(JAVA_DIR)/include \
	      -I $(JAVA_DIR)/include/linux \
	      -I $(STASIS_DIR) \
	      -L $(STASIS_DIR)/build/src/stasis \
	         src/stasis/jni/stasis_jni_Stasis.c \
	      -l stasis \
	      -o ant-build/stasis/jni/libstasisjni.so ; \
	else \
	  gcc -fPIC \
	      -I $(JAVA_DIR)/include \
	      -I $(STASIS_DIR) \
	       $(STASIS_DIR)/build/src/stasis/libstasis.dylib \
	       src/stasis/jni/stasis_jni_Stasis.c \
	       -dylib -o ant-build/stasis/jni/libstasisjni.dylib ; \
	fi

.PHONY: clean

clean : shutdown
	rm -rf ant-build/stasis/jni/Stasis.class \
	      src/stasis/jni/stasis_jni_Stasis.h \
	      ant-build/stasis/jni/libstasisjni.so \
	      apache-tomcat-6.0.18
	ant -q -e clean

.PHONY: checkvars
checkvars :
	@if [ ! -d "$(STASIS_DIR)" ] ; then  echo 'Define $$STASIS_DIR to continue!'; exit 1; fi
	@if [ ! -d "$(JAVA_DIR)" ] ; then  echo 'Define $$JAVA_DIR to continue!'; exit 1; fi


.PHONY: test
test : checkvars all
	LD_LIBRARY_PATH=$(STASIS_DIR)/build/src/stasis \
	java -cp ant-build -Djava.library.path=`pwd`/ant-build/stasis/jni stasis.jni.Stasis

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

.PHONY: deploy-stasis
deploy-stasis : checkvars all prepare-webapp-dir
	ant war -q -e
	cp ant-dist/lincoln.war apache-tomcat-6.0.18/webapps
	LD_LIBRARY_PATH=$(STASIS_DIR)/build/src/stasis/ \
	CATALINA_OPTS=-Djava.library.path=ant-build/stasis/jni \
	apache-tomcat-6.0.18/bin/startup.sh
	@echo "Deployed with Stasis"

.PHONY: deploy
deploy: prepare-webapp-dir
	ant war -q -e
	cp ant-dist/lincoln.war apache-tomcat-6.0.18/webapps
	apache-tomcat-6.0.18/bin/startup.sh
	@echo "Deployed without Stasis"

shutdown:
	@apache-tomcat-6.0.18/bin/shutdown.sh || true

startup: checkvars
	LD_LIBRARY_PATH=$(STASIS_DIR)/build/src/stasis/ \
	CATALINA_OPTS=-Djava.library.path=ant-build/stasis/jni \
	apache-tomcat-6.0.18/bin/startup.sh
