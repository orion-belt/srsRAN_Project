### Step 1. Build Docker Image ###

```bash
docker build -f docker/Dockerfile --build-arg UHD_VERSION=v4.3.0.0 --target srsran --tag srsran:latest .
```

### Step 2. Launch Tester ###
Make sure you have set up core-network already since we will reuse docker network)

```bash
docker-compose -f docker/docker-compose.yaml up -d
```

