DATE_TAG=$(shell date +%Y-%m-%d-%H%M%S)

help:
	@echo JustOJ Core & JustOJ Core Client

download-vendor:
	git clone https://github.com/ismdeep/ismdeep-c-utils.git vendor/ismdeep-c-utils
	git clone https://github.com/ismdeep/log.h.git           vendor/log.h

build-gcc-image:
	docker build -t ismdeep/ubuntu-gcc:$(DATE_TAG) -f ./dockerfiles/gcc/Dockerfile .
	docker push     ismdeep/ubuntu-gcc:$(DATE_TAG)
	docker tag      ismdeep/ubuntu-gcc:$(DATE_TAG) ismdeep/ubuntu-gcc:latest
	docker push     ismdeep/ubuntu-gcc:latest
	docker rmi      ismdeep/ubuntu-gcc:$(DATE_TAG)
	docker rmi      ismdeep/ubuntu-gcc:latest

create:
	-docker stop justoj-core-gcc
	-docker rm   justoj-core-gcc
	docker run --name justoj-core-gcc \
		-v $(CURDIR)/solutions:/solutions \
		-p 2222:22 -d ismdeep/ubuntu-gcc:latest

clean:
	-docker stop justoj-core-gcc
	-docker rm   justoj-core-gcc

bash:
	docker exec -it justoj-core-gcc bash
