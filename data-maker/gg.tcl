Mac/802_11 set dataRate_ 5.4e6 ;# rate for data frames in Mbps
set val(chan) Channel/WirelessChannel ;# channel type
set val(prop) Propagation/TwoRayGround ;# radio-propagation model
set val(netif) Phy/WirelessPhy ;# network interface type
set val(mac) Mac/802_11 ;# MAC type
set val(ifq) Queue/DropTail/PriQueue ;# interface queue type
set val(ll) LL ;# link layer type
set val(ant) Antenna/OmniAntenna ;# antenna model
set val(ifqlen) 50 ;# max packet in ifq
set val(rp) AODVUU ;# routing protocol
set val(x) 1300 ;# X dimension of topography
set val(y) 1300 ;# Y dimension of topography
set val(stop) 42 ;# nam stop time
set val(nn) 15 ;# number of mobilenodes
set val(nc) 3 ;# number of channels
set val(ni) 3 ;# number of interfaces, <= number of channels
set pktsize 1000 ;# packet size in bytes
set pktrate 0.01 ;# packet rate in seconds
set filename test
puts "Ad-Hoc Wireless Network in Chain Topologies - $val(nn) Nodes, $val(nc) Channels, $val(ni) Interfaces"
set ns_ [new Simulator]
set topo [new Topography]
$topo load_flatgrid $val(x) $val(y)
set god_ [create-god [expr $val(nn)*$val(nc)]]
set tracefd [open $filename.tr w]
$ns_ trace-all $tracefd
$ns_ use-newtrace
set namfile [open $filename.nam w]
$ns_ namtrace-all $namfile
$ns_ namtrace-all-wireless $namfile $val(x) $val(y)
for {set i 0} {$i < $val(nc)} {incr i} {
set chan($i) [new $val(chan)]
}
$ns_ node-config -adhocRouting $val(rp) \
-llType $val(ll) \
-macType $val(mac) \
-ifqType $val(ifq) \
-ifqLen $val(ifqlen) \
-antType $val(ant) \
-propType $val(prop) \
-phyType $val(netif) \
-topoInstance $topo \
-agentTrace ON \
-routerTrace ON \
-macTrace OFF \
-movementTrace OFF \
-ifNum $val(ni)    \
-workMode 0 \
-noiseChannel 0 
puts "begin to add channel"
$ns_ change-numifs $val(nc)
for {set i 0} {$i < $val(nc)} {incr i} {
  $ns_ add-channel $i $chan($i)
}
puts "begin to create nodes"
for {set i 0} {$i < 14} {incr i} {
  set n($i) [$ns_ node]
  $god_ new_node $n($i)
}
$ns_ node-config -workMode -1 \
-noiseChannel -1 
for {set i 14} {$i < 15} {incr i} {
  set n($i) [$ns_ node]
  $god_ new_node $n($i)
}
puts "created nodes"
set nodedist 250

$n(0) set X_ 700
$n(0) set Y_ 100
$n(0) set Z_ 0
$ns_ initial_node_pos $n(0) 50
$n(0) random-motion 0
$n(1) set X_ 700
$n(1) set Y_ 250
$n(1) set Z_ 0
$ns_ initial_node_pos $n(1) 50
$n(1) random-motion 0
$n(2) set X_ 500
$n(2) set Y_ 300
$n(2) set Z_ 0
$ns_ initial_node_pos $n(2) 50
$n(2) random-motion 0
$n(3) set X_ 300
$n(3) set Y_ 300
$n(3) set Z_ 0
$ns_ initial_node_pos $n(3) 50
$n(3) random-motion 0
$n(4) set X_ 200
$n(4) set Y_ 500
$n(4) set Z_ 0
$ns_ initial_node_pos $n(4) 50
$n(4) random-motion 0
$n(5) set X_ 400
$n(5) set Y_ 520
$n(5) set Z_ 0
$ns_ initial_node_pos $n(5) 50
$n(5) random-motion 0
$n(6) set X_ 200
$n(6) set Y_ 700
$n(6) set Z_ 0
$ns_ initial_node_pos $n(6) 50
$n(6) random-motion 0
$n(7) set X_ 300
$n(7) set Y_ 700
$n(7) set Z_ 0
$ns_ initial_node_pos $n(7) 50
$n(7) random-motion 0
$n(8) set X_ 900
$n(8) set Y_ 300
$n(8) set Z_ 0
$ns_ initial_node_pos $n(8) 50
$n(8) random-motion 0
$n(9) set X_ 900
$n(9) set Y_ 500
$n(9) set Z_ 0
$ns_ initial_node_pos $n(9) 50
$n(9) random-motion 0
$n(10) set X_ 950
$n(10) set Y_ 450
$n(10) set Z_ 0
$ns_ initial_node_pos $n(10) 50
$n(10) random-motion 0
$n(11) set X_ 925
$n(11) set Y_ 600
$n(11) set Z_ 0
$ns_ initial_node_pos $n(11) 50
$n(11) random-motion 0
$n(12) set X_ 700
$n(12) set Y_ 700
$n(12) set Z_ 0
$ns_ initial_node_pos $n(12) 50
$n(12) random-motion 0
$n(13) set X_ 500
$n(13) set Y_ 800
$n(13) set Z_ 0
$ns_ initial_node_pos $n(13) 50
$n(13) random-motion 0
$n(14) set X_ 100
$n(14) set Y_ 1300
$n(14) set Z_ 0
$ns_ initial_node_pos $n(14) 50
$n(14) random-motion 0

set udp0 [new Agent/UDP]
$ns_ attach-agent $n(0) $udp0
set sink0 [new Agent/Null]
$ns_ attach-agent $n(13) $sink0
$ns_ connect $udp0 $sink0
set cbr0 [new Application/Traffic/CBR]
$cbr0 attach-agent $udp0
$cbr0 set packetSize_ $pktsize
$cbr0 set interval_ $pktrate
$ns_ at 2.000000 "$cbr0 start"
$ns_ at 20.000000 "$cbr0 stop"

set udp1 [new Agent/UDP]
$ns_ attach-agent $n(4) $udp1
set sink1 [new Agent/Null]
$ns_ attach-agent $n(9) $sink1
$ns_ connect $udp1 $sink1
set cbr1 [new Application/Traffic/CBR]
$cbr1 attach-agent $udp1
$cbr1 set packetSize_ $pktsize
$cbr1 set interval_ $pktrate
$ns_ at 20.000000 "$cbr1 start"
$ns_ at 32.000000 "$cbr1 stop"

$ns_ at 8.000000 "$n(14) setdest 100 600 2000"
$ns_ at 8.500000 "$n(14) setdest 100 200 2000"
$ns_ at 12.000000 "$n(10) setdest 1250 500 2000"
$ns_ at 17.000000 "$n(14) setdest 100 600 2000"
$ns_ at 17.500000 "$n(14) setdest 100 1250 2000"

set last_node_id [expr $val(nn)-1]
proc finish {} {
global ns_ tracefd filename pktsize last_node_id
    global namfile
    $ns_ flush-trace
    close $tracefd
    close $namfile
    exec nam $filename.nam &
    exit 0
}
for {set i 0} {$i < $val(nn) } { incr i } {
    $ns_ at $val(stop) "$n($i) reset"
}
$ns_ at $val(stop) "$ns_ nam-end-wireless $val(stop)"
$ns_ at $val(stop) "finish"
$ns_ at $val(stop) "puts \"done\" ; $ns_ halt"
$ns_ run
