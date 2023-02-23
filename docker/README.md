### Step 1. Build Docker Image ###

```bash
docker build -f docker/Dockerfile --target srsran --tag srsran:latest .
```
[OPTIONAL] By defalut ubuntu focal is used as base image. We can also pass BASE_IMAGE as argument - <br/> e.g. to build with ubuntu jammy 
```bash
docker build --build-arg BASE_IMAGE=ubuntu:focal -f docker/Dockerfile --target srsran --tag srsran:latest .
```
### Step 2. Launch Tester ###
Make sure you have set up core-network already since we will reuse docker network)

```bash
docker-compose -f docker/docker-compose.yaml up -d
```

