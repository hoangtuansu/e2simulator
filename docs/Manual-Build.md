# Build and deploy in your own
## Prequitisites

```
export IMAGE_NAME=<your-repo>/<image-name>
export IMAGE_TAG=<tag-image>
export NAMESPACE=ran #change this to your own namespace
```

## Build
Make sure config template file is updated with actual values:

```
envsubst < helm/manifest.template.yaml > helm/e2sim.yaml
```

Run the following command to build and push to Docker registry:

```
docker build -t $IMAGE_NAME:$IMAGE_TAG . && docker push $IMAGE_NAME:$IMAGE_TAG
```

## Deploy

To deploy e2sim in K8s, run following command:

```
kubectl apply -f helm/e2sim.yaml -n $NAMESPACE
```