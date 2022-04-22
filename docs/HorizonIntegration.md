# How to integrate QOfflineMapServer with Micropilot Horizon GCS

First, follow the [README.md](../README.md) to download your map and start the QOfflineMapServer.

Then, open Micropilot Horizon and go to the `Settings` -> `Horizon Settings...`. If the text is grayed out, you have to select your UAV first.

![Horizon1](/docs/horizon1.png)

Go to `Maps` tab and check the `Enable downloading maps via custom tile server (specify {x} {y} {z})` checkbox. Then, paste the server address and port to the `URL` field.

**Note:** The server address should not contain any `$` characters. If it contains, simply remove them.

![Horizon2](/docs/horizon2.png)

Check `OSM/XYZ` radio button and select the tile extension according to the server address line.

Click `Save and Close`. Horizon will probably ask you to restart the application. Now you should be able to see your offline map.

![Horizon3](/docs/horizon3.png)


## Troubleshooting

- If you do not see any map, make sure that you are looking at the area you have downloaded.
- You can try to paste the server address into your browser and check that the image is displayed. Of course, you have to replace the `{x}`, `{y}`, `{z}` with numbers of the tile you want to see. You can find examples of these numbers in the folder with the downloaded map tiles.
