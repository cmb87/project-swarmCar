


module motor(){
    union(){
      translate([0,-19/2,0])cube([65,19,22+10]);  
      translate([11,37/2,11])rotate([90,0,0])cylinder(r=11/2,h=37);
      translate([65-25,-25/2,7])cube([25,25,13+30]);  
    }
}

module motorCasing(){
    union(){
      translate([-2,-19/2,0])cube([37,21,25-1.5]);  
    }
}


module frame(){
union(){
difference(){
    
  translate([500,-96.5/2,25])import("Motor_and_battery_part.stl");
    
  translate([-95+65,-70/2,2.5])cube([95,70,22+10]);
    
  translate([-7,-(39-2),2.5])color("red")motor();
  translate([-7,+(39-2),2.5])color("red")motor();
   
  for ( j = [-2,-1,0, 1,2] ){
      for ( i = [0 : 8] ){
        translate([-20+i*10,j*10,-1])cylinder(r=3/2,h=37, $fn=30);
      }
  }
}  
  
difference(){
translate([-5,-(39-2),2.5])color("green")motorCasing();
translate([-5,-(39-2),2.5])color("red")motor();
}

difference(){
translate([-5,+(39-2)-2,2.5])color("green")motorCasing();
translate([-5,+(39-2),2.5])color("red")motor();
}
}
}

frame();