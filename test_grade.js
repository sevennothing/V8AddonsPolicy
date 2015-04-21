/*
 * test_grade.js
 * Copyright (C) 2015 licj <licj@out.lrserver>
 *
 * Distributed under terms of the MIT license.
 */

var grade = require("./build/Release/seller_grade");



console.log("seller grade test start");

var arry = [0,1000,10,20, 1,10,5,20, 2,20,10,10, 3,2000,10,20, 4,1,0,0, 5,1,1,1, 6,100,0,1];

var res = grade.sort_seller_by_grade(arry,function(res){
	console.log("callback:" , res)		
});

console.log(res);

