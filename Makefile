help:
	@echo JustOJ Core & JustOJ Core Client

download-vendor:
	git clone https://github.com/ismdeep/ismdeep-c-utils.git vendor/ismdeep-c-utils
	git clone https://github.com/ismdeep/log.h.git           vendor/log.h

build-image:
	docker build -t gcc -f ./dockerfiles/gcc/Dockerfile .

create:
	-docker stop justoj-core-gcc
	-docker rm   justoj-core-gcc
	docker run --name justoj-core-gcc \
		-p 2222:22 -d gcc

clean:
	-docker stop justoj-core-gcc
	-docker rm   justoj-core-gcc


auto:
	-docker stop justoj-core-gcc
	-docker rm   justoj-core-gcc
	docker image prune -a -f
	make build-image
	make create
	docker ps

bash:
	docker exec -it justoj-core-gcc bash
