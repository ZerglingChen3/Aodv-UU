#include <bits/stdc++.h>

using namespace std;

//define CONSOLE
int Normal, Package, Noise, Move;
void init() {
    printf("Mac/802_11 set dataRate_ 5.4e6 ;# rate for data frames in Mbps\n");
    printf("set val(chan) Channel/WirelessChannel ;# channel type\n");
    printf("set val(prop) Propagation/TwoRayGround ;# radio-propagation model\n");
    printf("set val(netif) Phy/WirelessPhy ;# network interface type\n");
    printf("set val(mac) Mac/802_11 ;# MAC type\n");
    printf("set val(ifq) Queue/DropTail/PriQueue ;# interface queue type\n");
    printf("set val(ll) LL ;# link layer type\n");
    printf("set val(ant) Antenna/OmniAntenna ;# antenna model\n");
    printf("set val(ifqlen) 50 ;# max packet in ifq\n");
    printf("set val(rp) AODVUU ;# routing protocol\n");
    printf("set val(x) 1500 ;# X dimension of topography\n");
    printf("set val(y) 1500 ;# Y dimension of topography\n");
    printf("set val(stop) 42 ;# nam stop time\n");
    printf("set val(nn) %d ;# number of mobilenodes\n", Normal+Noise);
    printf("set val(nc) 3 ;# number of channels\n");
    printf("set val(ni) 3 ;# number of interfaces, <= number of channels\n");
    printf("set pktsize 1000 ;# packet size in bytes\n");
    printf("set pktrate 0.01 ;# packet rate in seconds\n");
    printf("set filename test\n");
    printf("puts \"Ad-Hoc Wireless Network in Chain Topologies - $val(nn) Nodes, $val(nc) Channels, $val(ni) Interfaces\"\n");

    printf("set ns_ [new Simulator]\n");
    printf("set topo [new Topography]\n");
    printf("$topo load_flatgrid $val(x) $val(y)\n");

    printf("set god_ [create-god [expr $val(nn)*$val(nc)]]\n");

    printf("set tracefd [open $filename.tr w]\n");

    printf("$ns_ trace-all $tracefd\n");
    printf("$ns_ use-newtrace\n");
    printf("set namfile [open $filename.nam w]\n");
    printf("$ns_ namtrace-all $namfile\n");
    printf("$ns_ namtrace-all-wireless $namfile $val(x) $val(y)\n");
    string forPattern = string("for {set i 0} {$i < $val(nc)} {incr i} {\n") +
                     "set chan($i) [new $val(chan)]\n" +
                     "}\n"; 
    printf(forPattern.c_str());
    string nsPattern = string("$ns_ node-config -adhocRouting $val(rp) \\\n") +
                       "-llType $val(ll) \\\n" +
                       "-macType $val(mac) \\\n" +
                       "-ifqType $val(ifq) \\\n" +
                       "-ifqLen $val(ifqlen) \\\n" +
                       "-antType $val(ant) \\\n" +
                       "-propType $val(prop) \\\n" +
                       "-phyType $val(netif) \\\n" +
                       "-topoInstance $topo \\\n" +
                       "-agentTrace ON \\\n" + 
                       "-routerTrace ON \\\n" +
                       "-macTrace OFF \\\n" +
                       "-movementTrace OFF \\\n" +
                       "-ifNum $val(ni)    \\\n" +
                       "-workMode 0 \\\n" +
                       "-noiseChannel 0 \n";
    printf(nsPattern.c_str());
    printf("puts \"begin to add channel\"\n");
    printf("$ns_ change-numifs $val(nc)\n");
    forPattern = string("for {set i 0} {$i < $val(nc)} {incr i} {\n") + 
                 "  $ns_ add-channel $i $chan($i)\n" + 
                 "}\n";
    printf(forPattern.c_str());
    printf("puts \"begin to create nodes\"\n");
    forPattern = string("for {set i 0} {$i < ") + to_string(Normal) + "} {incr i} {\n" +
                 "  set n($i) [$ns_ node]\n" +
                 "  $god_ new_node $n($i)\n" +
                 "}\n";
    printf(forPattern.c_str());
    nsPattern = string("$ns_ node-config -workMode -1 \\\n") +
                       "-noiseChannel -1 \n"; //this place we can change the disturb channel, now it set to 0
    printf(nsPattern.c_str());
    forPattern = string("for {set i ") + to_string(Normal) + "} {$i < " + to_string(Normal+Noise) + "} {incr i} {\n" +
                 "  set n($i) [$ns_ node]\n" +
                 "  $god_ new_node $n($i)\n" +
                 "}\n";
    printf(forPattern.c_str());
    printf("puts \"created nodes\"\n");
    printf("set nodedist 250\n");
}

void finish() {
    printf("set last_node_id [expr $val(nn)-1]\n");
    printf("proc finish {} {\n");
    printf("global ns_ tracefd filename pktsize last_node_id\n");
    printf("    global namfile\n");
    printf("    $ns_ flush-trace\n");
    printf("    close $tracefd\n");
    printf("    close $namfile\n");
    printf("    exec nam $filename.nam &\n");
    printf("    exit 0\n");
    printf("}\n");
    printf("for {set i 0} {$i < $val(nn) } { incr i } {\n");
    printf("    $ns_ at $val(stop) \"$n($i) reset\"\n");
    printf("}\n"); 
    printf("$ns_ at $val(stop) \"$ns_ nam-end-wireless $val(stop)\"\n");
    printf("$ns_ at $val(stop) \"finish\"\n");
    printf("$ns_ at $val(stop) \"puts \\\"done\\\" ; $ns_ halt\"\n");
    printf("$ns_ run\n");
}

int main() {
    #ifndef CONSOLE
    freopen("config.txt", "r", stdin);
    freopen("test.tcl", "w", stdout);
    #endif

    #ifdef CONSOLE
    scanf("?????????????????????????????????????????????????????????:\n%d%d%d\n", &Normal, &Package, &Noise, &Move);
    #else
    scanf("%d%d%d%d\n", &Normal, &Package, &Noise, &Move);
    #endif

    init();
    /*
    set points
    */
    printf("\n");
    for (int i = 0; i < Normal+Noise; ++ i) {
        int x, y, size = 25;
        #ifdef CONSOLE
        scanf("???????????????????????????:\n%d%d\n", &x, &y);
        #else
        scanf("%d%d\n", &x, &y);
        #endif
        printf("$n(%d) set X_ %d\n", i, x);
        printf("$n(%d) set Y_ %d\n", i, y);
        printf("$n(%d) set Z_ %d\n", i, 0);
        printf("$ns_ initial_node_pos $n(%d) %d\n", i, size);
        printf("$n(%d) random-motion %d\n", i, 0);
    }
    
    /*
    set packages
    */
    printf("\n");
    for (int i = 0; i < Package; ++ i) {
        int s, t;
        double T1, T2;
        char type[5];
        #ifdef CONSOLE
        scanf("?????????????????????????????????????????????????????????????????????????????????????????????????????????:\n%d%d%lf%lf%s\n", &s, &t, &T1, &T2, type);
        #else
        scanf("%d%d%lf%lf%s", &s, &t, &T1, &T2, type);
        #endif        
        if (type[0] == 'T') {
            string tcp = "tcp" + to_string(i);
            string sink = "sink" + to_string(i);
            string cbr = "cbr" + to_string(i);
            string sou = "$n(" + to_string(s) + ')';
            string tar = "$n(" + to_string(t) + ')';
            printf("set %s [new Agent/TCP]\n", tcp.c_str());
            printf("$ns_ attach-agent %s $%s\n", sou.c_str(), tcp.c_str());
            printf("set %s [new Agent/TCPSink]\n", sink.c_str());
            printf("$ns_ attach-agent %s $%s\n", tar.c_str(), sink.c_str());
            printf("$ns_ connect $%s $%s\n", tcp.c_str(), sink.c_str());
            printf("set %s [new Application/Traffic/CBR]\n", cbr.c_str());
            printf("$%s attach-agent $%s\n", cbr.c_str(), tcp.c_str());
            printf("$%s set packetSize_ $pktsize\n", cbr.c_str());
            printf("$%s set interval_ $pktrate\n", cbr.c_str());
            printf("$ns_ at %lf \"$%s start\"\n", T1, cbr.c_str());
            printf("$ns_ at %lf \"$%s stop\"\n", T2, cbr.c_str());
            printf("\n");
        } else {
            string udp = "udp" + to_string(i);
            string sink = "sink" + to_string(i);
            string cbr = "cbr" + to_string(i);
            string sou = "$n(" + to_string(s) + ')';
            string tar = "$n(" + to_string(t) + ')';
            printf("set %s [new Agent/UDP]\n", udp.c_str());
            printf("$ns_ attach-agent %s $%s\n", sou.c_str(), udp.c_str());
            printf("set %s [new Agent/Null]\n", sink.c_str());
            printf("$ns_ attach-agent %s $%s\n", tar.c_str(), sink.c_str());
            printf("$ns_ connect $%s $%s\n", udp.c_str(), sink.c_str());
            printf("set %s [new Application/Traffic/CBR]\n", cbr.c_str());
            printf("$%s attach-agent $%s\n", cbr.c_str(), udp.c_str());
            printf("$%s set packetSize_ $pktsize\n", cbr.c_str());
            printf("$%s set interval_ $pktrate\n", cbr.c_str());
            printf("$ns_ at %lf \"$%s start\"\n", T1, cbr.c_str());
            printf("$ns_ at %lf \"$%s stop\"\n", T2, cbr.c_str());
            printf("\n");
        }
    }

    /*
    set moves
    */
    for (int i = 0; i < Move; ++ i) {
        double tm;
        int pt, x, y, v;
        #ifdef CONSOLE
        scanf("??????????????????????????????????????????????????????????????????????????????:\n%lf%d%d%d%d\n", &tm, &pt, &x, &y, &v);
        #else
        scanf("%lf%d%d%d%d\n", &tm, &pt, &x, &y, &v);
        #endif          
        printf("$ns_ at %lf \"$n(%d) setdest %d %d %d\"\n", tm, pt, x, y, v);
    }
    printf("\n");

    finish();
    return 0;
}
