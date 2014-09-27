
var HIDDEN = 0x0010;
var BOM = 0x000F;

var createField = function (width, height)
{
	field = [];
	
	for (var y = 0; y < height; y++)
	{
		field[y] = [];
		for (var x = 0; x < width; x++)
		{
			field[y][x] = HIDDEN;
		}
	}
};

var outputField = function ()
{
	for (var y = 0; y < field.length; y++)
	{
		for (var x = 0; x < field[y].length; x++)
		{
			console.locate(x, y);
			if (x == cx && y ==cy)
			{
				console.color(console.G | console.I);
				console.print("*");
			}
			else if (field[y][x] & HIDDEN)
			{
				console.color(console.R | console.G | console.B | console.I);
				console.print("?");
			}
		}
	}
};


console.cls();
console.cursor(0);

var width = 10;
var height = 10;

var cx = width / 2;
var cy = height / 2;

var field;
createField(width, height);

console.locate(0, height + 1);
console.log("Q - Exit");
console.log("E - Up");
console.log("S - Left");
console.log("F - Right");
console.log("C - Down");

for (;;)
{
	if (console.key(0x51))
		break;
	if (0 < cy && console.key(0x45))
		cy--;
	if (cy < height - 1 && console.key(0x43))
		cy++;
	if (0 < cx && console.key(0x53))
		cx--;
	if (cx < width - 1 && console.key(0x46))
		cx++;
	outputField();
}
