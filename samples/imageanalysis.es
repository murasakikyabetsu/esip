IP =
{
	EDGE : [[-1, -1, -1], [-1, 8, -1], [-1, -1, -1]],
	SHARP : [[-1, -1, -1], [-1, 9, -1], [-1, -1, -1]],
	SMOOTH : [[1 / 9, 1 / 9, 1 / 9], [1 / 9, 1 / 9, 1 / 9], [1 / 9, 1 / 9, 1 / 9]],
	
	matrix : function (src, mat)
	{
		var dst = new ESIP.Image(src.width, src.height);
		
		var hmy = parseInt(mat.length / 2);
		var hmx = parseInt(mat[0].length / 2);
		
		// todo í[ÇÕê‹ÇËï‘Çµ
		for (var y = hmy; y < src.height - (mat.length - hmy); y++)
		{
			for (var x = hmx; x < src.width - (mat[0].length - hmx); x++)
			{
				var r = 0;
				var g = 0;
				var b = 0;
				for (var my = -hmy; my < -hmy + mat.length; my++)
				{
					for (var mx = -hmx; mx < -hmx + mat[0].length; mx++)
					{
						var pix = src.getPixel(x + mx, y + my);
						r += pix.r * mat[my + hmy][mx + hmx];
						g += pix.g * mat[my + hmy][mx + hmx];
						b += pix.b * mat[my + hmy][mx + hmx];
					}
				}
				dst.setPixel(x, y, r, g, b);
			}
		}
		
		return dst;
	},
	
	grayscale : function (src)
	{
		var dst = new ESIP.Image(src.width, src.height);

		var a = new Uint8Array(src.buffer);
		var b = new Uint8Array(dst.buffer);

		for (n = 0; n < a.length; n = n + 3)
		{
			var c = 0.299 * a[n + 2] + 0.587 * a[n + 1] + 0.114 * a[n + 0];
			b[n + 0] = c;
			b[n + 1] = c;
			b[n + 2] = c;
		}

		return dst;
	},
	
	show : function (img, title)
	{
		if (!title)
			title = "Image";
		
		return new Window.Image({
			text : title,
			width : img.width,
			height : img.height,
			src : img });
	}
}

var cmdWnd = new Window({
	text : "Toolbar",
	width : 120,
	height : 240,
	resizable : false,
	closeHandler : function ()
	{
		exit();
	},
	children : 
	[
		new Window.Button(
		{
			text : "Open",
			left : 0,
			top : 0,
			width : 120,
			height : 120,
			clickHandler : function ()
			{
				var path = Window.getOpenFileName();
				if (!path)
					return;
				var image = new ESIP.Image(path);
				
				var title = path.substring(path.lastIndexOf("\\") + 1, path.lastIndexOf("."));
				
				if (Global[title])
				{
					Global[title].text = title;
					Global[title].src = image;
				}
				else
					Global[title] = IP.show(image, title);
			}
		}),
		new Window.Button(
		{
			text : "Exit",
			left : 0,
			top : 120,
			width : 120,
			height : 120,
			clickHandler : function ()
			{
				exit();
			}
		})
	]});

