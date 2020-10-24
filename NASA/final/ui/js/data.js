var	margin = {top: 30, right: 20, bottom: 30, left: 80},
	width = 700 - margin.left - margin.right,
	height = 220 - margin.top - margin.bottom;

// Parse the date / time
var parsetime = d3.time.format("%Y-%m-%d %X").parse;

// Set the ranges
var	x = d3.time.scale().range([0, width]);
var	y = d3.scale.linear().range([height, 0]);

// Define the axes
var	xAxis = d3.svg.axis().scale(x)
	.orient("bottom").ticks(5);

var	yAxis = d3.svg.axis().scale(y)
	.orient("left").ticks(5);

// Define the line
var	valueline1 = d3.svg.line()
	.x(function(d) { return x(d.time); })
	.y(function(d) { return y(d.packetloss); });
var	valueline2 = d3.svg.line()
	.x(function(d) { return x(d.time); })
	.y(function(d) { return y(d.latency); });
var	valueline3 = d3.svg.line()
	.x(function(d) { return x(d.time); })
	.y(function(d) { return y(d.throughput); });
	
// Adds the svg canvas
var	chart1 = d3.select("#chart")
	.append("svg")
		.attr("id","chart1")
		.attr("width", width + margin.left + margin.right)
		.attr("height", height + margin.top + margin.bottom)
	.append("g")
		.attr("transform", "translate(" + margin.left + "," + margin.top + ")");

var	chart2 = d3.select("#chart")
	.append("svg")
		.attr("id","chart2")
		.attr("width", width + margin.left + margin.right)
		.attr("height", height + margin.top + margin.bottom)
	.append("g")
		.attr("transform", "translate(" + margin.left + "," + margin.top + ")");

var	chart3 = d3.select("#chart")
	.append("svg")
		.attr("id","chart3")
		.attr("width", width + margin.left + margin.right)
		.attr("height", height + margin.top + margin.bottom)
	.append("g")
		.attr("transform", "translate(" + margin.left + "," + margin.top + ")");

var initChart = function(chart, value, yLabel){
	chart.append("path")
		.attr("class", "line")
		.attr("d", value);

	// Add the X Axis
	chart.append("g")
		.attr("class", "x axis")
		.attr("transform", "translate(0," + height + ")")
		.style("text-anchor", "middle")
        .text("Time")
		.call(xAxis);

	// Add the Y Axis
	chart.append("g")
		.attr("class", "y axis")
		.call(yAxis);
	
	chart.append("text")
      .attr("transform", "rotate(-90)")
      .attr("y", 0 - margin.left)
      .attr("x",0 - (height / 2))
      .attr("dy", "1em")
      .style("text-anchor", "middle")
      .text(yLabel); 
	
}

function updateChart(chart, value) {
	chart.select(".line")
	     .attr("d", value);
	
	chart.select(".x.axis")
		 .call(xAxis);
	
	chart.select(".y.axis")
		 .call(yAxis);
}

var curCSV = "metric0.in";

// Get the data
d3.csv(curCSV, function(error, data) {

	data.forEach(function(d) {
		d.time = parsetime(d.time);
	});

	// Scale the range of the data
	x.domain(d3.extent(data, function(d) { return d.time; }));
	y.domain([0, 100]);
	// Add the valueline path.
	initChart(chart1, valueline1(data), "packetLoss(%)");
	
	y.domain([0, d3.max(data, function(d) { return Math.max(1, +d.latency); })]);
	initChart(chart2, valueline2(data), "rtt(ms)");
	
	y.domain([0, d3.max(data, function(d) { return Math.max(1, +d.throughput); })]);
	initChart(chart3, valueline3(data), "throughput(KB/s)");
	
});


var updateData = function(){
	// Get the data again
	d3.csv(curCSV, function(error, data) {
	   	data.forEach(function(d) {
			d.time = parsetime(d.time);
		});

		// Scale the range of the data again 
		x.domain(d3.extent(data, function(d) { return d.time; }));
		y.domain([0, 100]);
		// Select the section we want to apply our changes to
		updateChart(chart1, valueline1(data));
		
		y.domain([0, d3.max(data, function(d) { return Math.max(1, +d.latency); })]);
		updateChart(chart2, valueline2(data));
		
		y.domain([0, d3.max(data, function(d) { return Math.max(1, +d.throughput); })]);
		updateChart(chart3, valueline3(data));
	});
}

var inter = setInterval(updateData, 60000); 

var openMetric = function(link_id) {
	curCSV = "/nasa/ui/metric"+link_id+".in";
	updateData();
};


