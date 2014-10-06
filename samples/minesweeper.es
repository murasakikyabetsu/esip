
var HIDDEN = 0x0010;
var FLAG = 0x0020;
var BOM = 0x000F;

var createField = function (width, height, boms)
{
	var field = [];
	
	for (var y = 0; y < height; y++)
	{
		field[y] = [];
		for (var x = 0; x < width; x++)
		{
			field[y][x] = 0;
		}
	}
	
	for (var n = 0; n < boms; n++)
	{
		while (1)
		{
			var x = parseInt(Math.random() * width);
			var y = parseInt(Math.random() * height);
			
			if (field[y][x] == BOM)
				continue;
			
			field[y][x] = BOM;
			
			for (var yy = y - 1; yy <= y + 1; yy++)
			{
				if (yy < 0 || height <= yy)
					continue;
				
				for (var xx = x - 1; xx <= x + 1; xx++)
				{
					if (xx < 0 || width <= xx)
						continue;
					
					if (field[yy][xx] == BOM)
						continue;
					
					field[yy][xx]++;
				}
			}
			
			break;
		}
	}

	for (var y = 0; y < height; y++)
	{
		for (var x = 0; x < width; x++)
		{
			field[y][x] |= HIDDEN;
		}
	}
	
	return field;
};

var outputField = function (field)
{
	console.color(console.R | console.G | console.B | console.I);
	console.locate(0, 0);
	console.print("+");
	for (var x = 0; x < field[0].length; x++)
	{
		console.print("-");
	}
	console.print("+");
	
	for (var y = 0; y < field.length; y++)
	{
		console.locate(0, y + 1);
		console.color(console.R | console.G | console.B | console.I);
		console.print("|");
		
		for (var x = 0; x < field[y].length; x++)
		{
			console.locate(x + 1, y + 1);
			if (x == cx && y ==cy)
			{
				console.color(console.G | console.I);
				console.print("@");
			}
			else if (field[y][x] & FLAG)
			{
				console.color(console.R | console.G | console.I);
				console.print("P");
			}
			else if (field[y][x] & HIDDEN)
			{
				console.color(console.R | console.G | console.B | console.I);
				console.print("?");
			}
			else
			{
				if (field[y][x] == BOM)
				{
					console.color(console.R | console.I);
					console.print("*");
				}
				else if (field[y][x] == 0)
				{
					console.color(console.R | console.G | console.B | console.I);
					console.print(" ");
				}
				else
				{
					console.color(console.B | console.I);
					console.print(field[y][x]);
				}
			}
		}
		
		console.locate(width + 1, y + 1);
		console.color(console.R | console.G | console.B | console.I);
		console.print("|");
	}

	console.color(console.R | console.G | console.B | console.I);
	console.locate(0, height + 1);
	console.print("+");
	for (var x = 0; x < field[0].length; x++)
	{
		console.print("-");
	}
	console.print("+");
};

var showExplanation = function ()
{
	console.locate(0, height + 3);
	console.log("E - Up");
	console.log("S - Left");
	console.log("F - Right");
	console.log("C - Down");
	console.log("");
	console.log("Z - Open");
	console.log("X - Flag");
	console.log("");
	console.log("A - Retry");
	console.log("Q - Exit");
}

var open = function (field, x, y)
{
	if (!(field[y][x] & HIDDEN) || (field[y][x] & FLAG))
		return;
	
	field[y][x] &= ~HIDDEN;
	if (field[y][x] == BOM)
	{
	}
	else if (field[y][x] == 0)
	{
		for (var yy = y - 1; yy <= y + 1; yy++)
		{
			if (yy < 0 || field.length <= yy)
				continue;
			
			for (var xx = x - 1; xx <= x + 1; xx++)
			{
				if (xx < 0 || field.length <= xx)
					continue;
				
				open(field, xx, yy);
			}
		}
	}
}

var check = function (field)
{
	var count = 0;
	for (var y = 0; y < field.length; y++)
	{
		for (var x = 0; x < field[y].length; x++)
		{
			if (field[y][x] & HIDDEN)
				count++;
			
			if (field[y][x] == BOM)
				return 2;
		}
	}
	
	if (count == boms)
		return 1;
	
	return 0;
}

var width = 10;
var height = 10;
var boms = 10;

var cx = parseInt(width / 2);
var cy = parseInt(height / 2);

var field = createField(width, height, boms);

var keyState = {};
var keyUp = function (keyCode)
{
	var state = console.key(keyCode);
	var keyUp = !state && keyState[keyCode];
	keyState[keyCode] = state;
	return keyUp;
}

console.cursor(0);
console.cls();
showExplanation();

var prevTime = (new Date()).getTime();

for (;;)
{
	if (keyUp(0x51))
		break;
	if (keyUp(0x41))
	{
		field = createField(width, height, boms);
		console.cls();
		showExplanation();
		cx = parseInt(width / 2);
		cy = parseInt(height / 2);
	}
	
	var ret = check(field);
	if (ret == 1)
	{
		outputField(field);
		
		console.locate(width + 4, parseInt(height / 2));
		console.log("+-------+");
		console.locate(width + 4, parseInt(height / 2) + 1);
		console.log("| Clear |");
		console.locate(width + 4, parseInt(height / 2) + 2);
		console.log("+-------+");
	}
	else if (ret == 2)
	{
		outputField(field);
		
		console.locate(width + 4, parseInt(height / 2));
		console.log("+-----------+");
		console.locate(width + 4, parseInt(height / 2) + 1);
		console.log("| Game Over |");
		console.locate(width + 4, parseInt(height / 2) + 2);
		console.log("+-----------+");
	}
	else
	{
		if (0 < cy && keyUp(0x45))
			cy--;
		if (cy < height - 1 && keyUp(0x43))
			cy++;
		if (0 < cx && keyUp(0x53))
			cx--;
		if (cx < width - 1 && keyUp(0x46))
			cx++;
		if (keyUp(0x5A))
			open(field, cx, cy);
		if (keyUp(0x58))
		{
			if (field[cy][cx] & FLAG)
				field[cy][cx] &= ~FLAG;
			else if (field[cy][cx] & HIDDEN)
				field[cy][cx] |= FLAG;
		}
	
		outputField(field);
	}
	
	var curTime = (new Date()).getTime();
	if (curTime - prevTime < 25)
	{
		console.sleep(25 - (curTime - prevTime));
	}
	prevTime = curTime;
}
