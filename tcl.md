```tcl
set val(chan) Channel/WirelessChannel ;# 信道类型
set val(prop) Propagation/TwoRayGround ;# radio-propagation model
set val(netif) Phy/WirelessPhy ;# network interface type
set val(mac) Mac/802_11 ;# MAC type
set val(ifq) Queue/DropTail/PriQueue ;# interface queue type
set val(ll) LL ;# link layer type
set val(ant) Antenna/OmniAntenna ;# antenna model
set val(ifqlen) 50 ;# max packet in ifq
set val(rp) AODVUU ;# routing protocol
set val(x) 1000 ;# X 边界值
set val(y) 1000 ;# Y 边界值
set val(stop) 42 ;# nam stop time 仿真时间
set val(nn) 5 ;# 节点数量
set val(nc) 3 ;# 信道数量
set val(ni) 3 ;# 网卡数量（一般一个节点对应一个）, <= number of channels
set pktsize 1000 ;# 发包的大小
set pktrate 0.01 ;# 发包的频率 
```

### 节点参数设置

```tcl
#======================================
# Mobile Node Parameter Setup
#======================================
$ns_ node-config -adhocRouting $val(rp) \# routing protocol
                -llType $val(ll) \# link layer type
                -macType $val(mac) \
                -ifqType $val(ifq) \# interface queue type
                -ifqLen $val(ifqlen) \# max packet in ifq
                -antType $val(ant) \# antenna model
                -propType $val(prop) \# radio-propagation model
                -phyType $val(netif) \# network interface type
                -channel $chan \
                -topoInstance $topo \# 设定拓扑图的大小
                -agentTrace ON \
                -routerTrace ON \
                -macTrace OFF \
                -numifs 3\ # channel 数
                -movementTrace OFF \
                -ifNum $val(ni)	\# 接口数？
				-workMode 0 \
				-noiseChannel 0
```

### 创建节点

```tcl
#======================================
# Nodes Definition
#======================================
puts "begin to add channel"
$ns_ change-numifs $val(nc)
for {set i 0} {$i < $val(nc)} {incr i} {
    $ns_ add-channel $i $chan($i)
}
# 创建节点
puts "begin to create nodes"
for {set i 0} {$i < $val(nn)} {incr i} {
    set n($i) [$ns_ node]
    $god_ new_node $n($i)
}
puts "created nodes"
# 设置节点的位置
set nodedist 250

$n(0) set X_ 100
$n(0) set Y_ 300
$n(0) set Z_ 0
$ns_ initial_node_pos $n(0) 50
$n(0) random-motion 0

$n(1) set X_ 300
$n(1) set Y_ 300
$n(1) set Z_ 0
$ns_ initial_node_pos $n(1) 50
$n(1) random-motion 0

$n(2) set X_ 500
$n(2) set Y_ 300
$n(2) set Z_ 0
$ns_ initial_node_pos $n(2) 50
$n(2) random-motion 0

$n(3) set X_ 750
$n(3) set Y_ 300
$n(3) set Z_ 0
$ns_ initial_node_pos $n(3) 50
$n(3) random-motion 0

$n(4) set X_ 450
$n(4) set Y_ 600
$n(4) set Z_ 0
$ns_ initial_node_pos $n(4) 50
$n(4) random-motion 0


#======================================
# Agents Definition
#======================================
set udp0 [new Agent/UDP]
$ns_ attach-agent $n(0) $udp0
set sink0 [new Agent/Null]
set last_node_id [expr $val(nn)-1]
$ns_ attach-agent $n(3) $sink0
$ns_ connect $udp0 $sink0

#======================================
# Applications Definition
#======================================
# Setup a CBR Application over UDP connection
set cbr0 [new Application/Traffic/CBR]
$cbr0 attach-agent $udp0
$cbr0 set packetSize_ $pktsize
$cbr0 set interval_ $pktrate
$ns_ at 1.0 "$cbr0 start"
$ns_ at 20.0 "$n(4) setdest 450 300 800"
$ns_ at 20.0 "$n(2) setdest 600 300 400"

$ns_ at 41.0 "$cbr0 stop"
```

### 执行仿真

```tcl
#======================================
# Simulation Termination
#======================================
# Define a finish procedure
proc finish {} {
global ns_ tracefd filename pktsize last_node_id
    global namfile
    $ns_ flush-trace
    close $tracefd
    close $namfile
    exec nam $filename.nam &
    # Call throughput analyzer (AWK scripts written by Marco Fiore, marco.fiore@polito.it)
    exit 0
}
for {set i 0} {$i < $val(nn) } { incr i } {
    $ns_ at $val(stop) "\$n($i) reset"
} 
$ns_ at $val(stop) "$ns_ nam-end-wireless $val(stop)"
$ns_ at $val(stop) "finish"
$ns_ at $val(stop) "puts \"done\" ; $ns_ halt"
$ns_ run
```

