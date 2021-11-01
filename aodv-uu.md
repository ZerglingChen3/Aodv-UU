### defs.h

定义了一些基本信息（比如 aodvuu 版本信息，说明文档名字等），定义了两个结构体：

- dev_info：主要是当前机器可用的 network device（网络设备，可以理解为网卡） 的信息，包括状态 enabled、socket、接口名字 ifname、接口序号 ifindex、ip 地址、网络掩码 netmask、广播地址 broadcast；
- host_info：当前结点的（最近一次使用的）序列号、上次广播时间 bcast_time、上次发包时间 fwd_time、RREQ 序列号 rreq_id、网络接口数目 nif、网卡信息 devs。

其中定义时间用的是 timeval 这个结构体，它是一个包含 x 秒 y 微妙（这两个域）的一个简单结构体；

定义地址用的是 in_addr 这个结构体，它是这样一个结构体：

```c
struct in_addr {
        union {
                struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { u_short s_w1,s_w2; } S_un_w;
                u_long S_addr;
        } S_un;
```

这是C++自带的一个表示IPV4地址的结构体。

除此之外，该文件还定义了如下一些东西：

- MAX_NR_INTERFACE：最大网络接口数
- dev_indices[MAX_NR_INTERFACE]：网卡信息的数组
- ifindex、devindex、dev、name 之间的各种等价类型转换
- DEV_IFINDEX(ifindex)、DEV_NR(n)：获取 dev 信息（也就是 dev 的 dev_info 结构体）的比较方便的宏定义
- AODV_BROADCAST：广播地址 ((in_addr_t) 0xFFFFFFFF)
- AODV 信息种类：AODV_HELLO(0), AODV_RREQ(1), AODV_RREP(2), AODV_RERR(3), AODV_RREP_ACK(4)
- AODV_msg：这是一个信息类别，所有的 RREQ、RREP 等等消息在发送前都会转换为这个消息，收到这个消息时可以根据前面的类型再转换为对应的具体的消息类别
- extension：不知道干嘛的，说是可以给消息附加邻居信息？
- callback function：定义了一个带有一个 int 参数的函数指针 callback_func_t

***

### routing_table.{h, c}

定义了一个序列号增加的宏 seqno_incr：0 就保持为 0，其它数值在 1-0xFFFFFFFF 之间循环；

定义了路由表项（route table entry）结构体 rt_table（rt_table_t），包含以下内容：

- l：链表域
- dest_addr：目标地址
- dest_seqno：目标序列号
- ifindex：网络接口序号？干啥用的
- next_hop：下一跳的地址
- hcnt：到达目标地址所需要的跳数
- flags：路由标识，用来标识是前向（forward）、反向（reverse）、邻居（neighbor）、单向（uni-directional）还是路径修复（local repair）?
- flags：路由标识，用来标识单向（uni-directional **0x1**）、路径修复（local repair **0x2**）、网络终点（Internet destinations **0x8**）、网关(GateWay **0x10**)
- state：当前路由表项的状态（VALID or INVALID）
- rt_timer：当前路由表项的计时器，是一个 timer 结构体（这个之后会讨论到），它绑定的 callback 函数是 aodv_timeout.c 里面的 route_expire_timeout()，作用是如果对应路由表项是邻居节点，那么调用 neighbor_link_break，否则使当前的路由项无效（**总体过程就是对路由到期的处理**）。
- ack_timer：RREP_ack 的计时器，绑定的 callback 是 rrep_ack_timeout()，作用是把该路由表项的目的节点地址加入黑名单（之后会讨论到）
- hello_timer：hello 消息的计时器，绑定的 callback 是 hello_timeout()
- last_hello_time：上一个hello消息的时间单元
- hello_cnt：收到的hello消息数量？
- hash：一个哈希值，用来快速定位这个路由表项
- nprec：先驱节点（也就是这样的节点，它们也有到达同样目标地址的路由表项，并且下一跳是这个节点）数目
- precursors：这是一个链表表头，用来存储先驱节点地址信息

定义了一个路由表的变量 rt_tbl，它是一个这样的结构体：包含了表项数目、活跃节点数目、路由表项数组 tbl（也是一个链表）。

定义了若干个函数：

- rt_table_init
  * 初始化路由表，设置路由表项和活跃路由表项个数都设置为0
- rt_table_destroy
  * 删除整个路由表
- rt_table_insert
  - 检查路由表项是否已经存在，如果已经存在，返回NULL
  - 根据传入的参数设置路由表项的各种值
  - 初始化计时器(life)
  - 插入路由表项
  - 对于插入非有效的路由表项时，需要考虑是修复或者删除问题
- rt_table_update
  * 更新路由表项
  * 要考虑旧的路由表项是否为非有效，且新的路由表项是否有效
  * 从邻居节点变成非邻居节点时修改hello相关信息
  * **对于insert和update都有一个buffer的问题，但是我不太理解？**
- rt_table_update_timeout
  * 更新路由表项的lifetime
  * 注意要比较lifetime和原本的时间，只有时间最长的才保留
- rt_table_update_route_timeouts
  * 对于到来的和要转发出去的数据包，更新路由的lifetime
- rt_table_find
  * 根据传入的路由表项的目的地址，返回对应的路由表项
- rt_table_find_gateway
  - 查找当前路由表中GateWay中跳数最少的
- rt_table_invalidate
  * 将路由表项的 state 记为 INVALID
  * 删除所有该路由表项的计时器
  * 生成一个 local_repair_timeout 或者 route_delete_timeout 的 timer
- rt_table_delete
  * 删除一个路由表项
- precursor_add
  * 在前驱节点列表中加入一个新的路由表项
- precursor_remove
  * 从前驱节点列表中删除一个路由表项
- precursor_list_destroy
  * 删除整个前驱节点列表

其实无非就是插入、删除、修改路由表的操作，还有前驱节点的更新操作，结合路由表项的结构体应该很容易理解这些函数的实现。

另外补充一点，rt_tbl（routing_table）里面的 tbl 数组实际上是一个 hash 表，每个路由表项会获得一个哈希值，然后 tbl 数组的每个元素都是一个链表头（这实际上就是处理 hash 冲突的一种方式），这样可以加速路由表项的查找（从中我们可以推断路由表项可能会比较多，并且查找路由的操作也很频繁）

***

### timer_queue.{h, c}

定义了一个带有一个 void 指针的函数指针 timeout_func_t，然后定义了 timer 结构体：

- l：链表域
- used：主要是用来记录该 timer 是否已经进入队列，当我们在添加一个 timer 的时候，如果它已经进入了队列，首先要把它删除，然后再添加
- timeout：记录时间的 timeval 变量
- handler：记录时间到了的时候调用的处理函数指针（类型为 timeout_func_t）
- data：一个 void 类型的指针，存储一些 handler 需要的数据

之后定义了一些针对 timeval 的计算的函数，包括 timeval_diff（时间差）、timeval_add_msec（增加时间）等等。

然后定义了 timer_queue 的一些相关操作，对于 timer queue，可以理解为一个 timer 的链表，这个链表是有序的，按照每个 timer 的发生顺序**从近到远**排序，这样离散事件模拟的时候每次取链表头的事件来处理就行了。相关操作如下：

- timer_queue_init：未实现（不过在 timer_queue.c 里面定义了一个链表头 TQ）
- timer_init
  * 用来初始化一个 timer
  * 对于不同的时钟设置不同的处理函数等
- timer_timeout
  * 当一个 timer 到时的时候调用的函数
  * 把某个时间前的所有 timer 都移除，并且调用他们的处理函数
- timer_add
  * 如果已经 used，首先删除，然后添加
  * 在添加的时候遍历整个 TQ，找到合适的位置插入（保证时序）
- timer_remove
  * 在 TQ 链表中删除指定 timer
- timer_timeout_now
  * 删除当前 timer
  * 调用处理函数（不知道这个有啥用）
- timer_set_timeout
  * 改变某个 timer 的 timeout（如果这个 timer 已经进入队列也就是 used 了，首先要移出）
- timer_left
  * 返回某个timer的剩余时间
- time_age_queue
  * 首先把当前时间（now）之前的 timer 全部到时（也就是调用 timer_timeout(&now)），
  * 返回下一个即将到期的 timer 剩下的时间

总览整个 timer_queue 的功能，其实就是一个离散事件模拟器，用一个链表记录即将到来的事件，然后对相应的事件调用相应的处理函数，需要计时的比如 hello 消息的统计都会通过这个方式来完成。另外里面还有一个 gettimeofday 函数，用来获取当前时间，它实际上是获取了调度器 Scheduler 的时钟。

***

### seek_list.{h, c}

这个文件是用来实现 seek list 的，seeklist 记录的是寻路的队列（有点儿神奇），实现了一个寻路请求的结构体 seek_list_t 以及寻路队列链表头 seedhead，该队列支持插入、删除、（通过 dest_addr）查找的操作。其中每个 entry（seek_list_t）的结构体包含如下内容：

- l：链表域
- dest_addr：寻路目的节点的地址
- dest_seqno：目的节点的序列号
- ipd：一个 ip_data 类型的数据，记录一个地址，当寻路失败时向这个地址发送目的主机不可达的 ICMP 报文
- flags：一个用来重发送 RREQ 的标识
- reqs：已经发送 RREQ 的数目
- ttl：发送 RREQ 时使用的 TTL 值
- seek_timer：寻路使用的计时器，绑定的处理函数是 route_discovery_timeout，这个函数的功能是：如果 reqs 小于 RREQ_RETRIES（定义与 params.h），那么 reqs 自增，ttl 增加（增加方式也已经说过），然后重新设置 seek_timer，时间为 RING_TRAVERSAL_TIME，然后更新路由表中相应目的节点表项的 timeout（表示，啊，我要重新发送 RREQ 了，所以你还得再等 2 * NET_TRAVERSAL_TIME 的时间才能从路由表中删去），然后发送 RREQ；否则的话，表示已经达到了一次寻路所限制的最大的发送 RREQ 数目限制，那么就认为寻路失败，寻路失败首先调用了一个 nl 相关的函数发送一些寻路失败信息，然后移出当前 seek 表项，然后从路由表找到目的节点路由信息（可能已经失效但是还没有移除），如果该表项的 flags 标识了 RT_REPAIR，那么调用 local_repair_timeout（表示当前发送 RREQ 是为了路由修复，然后修复失败了，它的作用主要是广播 RERR 信息，然后生成一个删除路由的 timer，绑定 route_delete_timeout 函数，然后在 DELETE_PERIOD 之后删除该路由）

插入、删除和查找的三个函数都非常简单，很容易理解。

***

### aodv_neighbor.{h, c}

主要也是基于 routing_table 下的操作，里面有两个函数：

- neighbor_add
  * 这个是给非 HELLO 的控制消息更新邻居用的，其实就是对 routing_table 操作一下，然后更新一下 hello_timer
- neighbor_link_break
  * 邻居链接中断
  * 首先 invalidate 一下路由表项
  * 如果该路由表项不是 RT_REPAIR 并且有前驱节点，会生成一个 RERR 信息
  * 遍历所有路由表项，找到下一跳地址等于该邻居地址的路由表项
  * 如果当前邻居路由表项标识了 RT_REPAIR，则也把遍历路由表项标识为 RT_REPAIR，然后 invalidate 该遍历路由表项，然后如果该遍历路由表项有前驱节点，还会在 RERR 消息上附加 udest 消息。最后发送 RERR 消息。

顺便提一句，在 aodv 里面判断某个路由是不是记录邻居节点信息，是通过 hcnt 是否等于 1 来判断的（显然）。

***

### aodv_hello.{h, c}

在 aodvuu 协议中，节点-4会定时广播一个 hello 消息用于维护和邻居的拓扑性质，具体体现在，aodv_hello.c 里面定义了一个 hello 专属的 timer：hello_timer，它初始化的时候会绑定一个 hello_send 函数，然后调用 hello_send。每次执行 hello_send 时，在最后又会给 hello_timer 重新 timer_set_timeout 一下，相当于再次设置了一个定时，这样实现了定时发送的功能（这个定时其实不准确，因为实际上它还在 HELLO_INTERVAL 上加了个 jitter）。

另外 hello 消息实际上是一个 RREP 消息。

这两个文件里面定义了如下一些函数：

- hello_jitter
  - 如果hello_jittering为真则产生一个抖动的值，在[-50, 50]之间
  - 否则不产生抖动
- hello_start
  * 初始化 hello_timer
  * 调用hello_send函数进行消息的发送
- hello_stop
  * 从 timer_queue 中移除 hello_timer，相当于停止定时发送 hello 消息
- hello_send
  - 首先如果optimized_hellos是真并且当前hello消息的存活时间超过生命周期，直接调用hello_stop进行消息的自毁
  - 首先要在当前时间减去上一次广播时间大于等于 HELLO_INTERVAL 时才进行 HELLO 消息的发送（目的是防止广播太过于频繁）
  - 发送 hello 消息前首先遍历所有接口，然后生成一个目的节点地址和序列号都是自己的 RREP，lifetime 设置为 ALLOWED_HELLO_LOSS * HELLO_INTERVAL，然后进行广播
  - 发送完之后重新给 hello_timer 进行 timer_set_timeout（里面还有一个 ext 消息，这里不作讨论）。
  - 时间有可能是负的吧。
- hello_process
  * 对 hello 消息进行处理的函数，其中最核心的是对路由表的更新吧（包括更新路由表项的 hello_timer、last_hello_time 等）
- hello_process_non_hello
- hello_update_timeout
  * 在 hello_process 最后会调用这个函数，更新路由表项里面的 hello_timer

总而言之，这两个文件是专门用来处理 hello 消息的，而 hello 消息的作用是用来维护邻居信息，主要更新和操作的是路由表。

hello消息后面有不止一个扩展域ext，由ext->type来决定消息类型。

- RREP_HELLO_INTERVAL_EXT：记录Hello时间间隔的，长度为4位。

- RREP_HELLO_NEIGHBOR_SET_EXT：记录邻居信息，ext->length为邻居个数

***

### aodv_rreq.{h, c}

定义了 RREQ 消息的格式：

- type：消息类型，一般赋值为 AODV_RREQ，这个是用来匹配 AODV_msg 的
- j, r, g, d：每个各占一位，用来标识 RREQ_JOIN，RREQ_REPAIR，RREQ_GRATUITOUS，RREQ_DEST_ONLY
- res1, res2：保留字段
- hcnt：记录当前经过的跳数
- rreq_id：这个 RREQ 的 id
- dest_addr：目的节点地址
- dest_seqno：目的节点序列号
- orig_addr：源节点地址
- orig_seqno：源节点序列号

![img](https://dragonylee-blog.oss-cn-beijing.aliyuncs.com/markdown/aodvuu_rreq_1.jpg)

RREQ 消息格式

定义了 rreq_record 结构体（包含源节点地址、rreq_id 和一个 rec_timer），并且定义了该结构体的链表头 rreq_records，这个的作用相当于对收到的 RREQ 消息进行缓存。rec_timer 绑定的处理函数是 rreq_record_timeout，该函数的作用就仅仅是把这个 record 从链表中清除，timeout 设置为 PATH_DISCOVERY_TIME。

定义了 blacklist 结构体以及 rreq_blacklist 链表头，顾名思义这是一个黑名单，这个黑名单的作用是这样的（这是我的理解）：由于路由可能是单向的，所以一个节点收到 RREQ 之后可能无法返还 RREP 消息，而每个节点对于同一个 rreq_id 只会处理第一个，这就导致在寻路的时候可能存在合法的双向路径，但是被忽略了。所以当一个节点发送 RREP 消息而不被下一跳接收到时，就会把下一跳结点的地址加入黑名单，表示之后的一段时间内（BLACKLIST_TIMEOUT），都不接受它的 RREQ 消息，这就解决了上述问题。而 BLACKLIST_TIMEOUT 的时间设置应当为允许多次寻路之后的时间上限。所以加入黑名单只在等不到 RREP_ack 消息的时候进行，也就是说只有 rrep_ack_timeout 会调用它。

定义了若干个函数：

- rreq_create
  * 生成并返回一个 RREQ 消息结构体，生成的时候自己的 seqno 要加一
- rreq_send
  * 遍历所有接口
  * 生成 RREQ 消息
  * 调用 aodv_socket_send 进行广播（广播只需要把 aodv_socket_send 的 dest 地址设置为 AODV_BROADCAST）
- rreq_forward
  * 把 RREQ 消息里面的 hcnt 加一，然后遍历所有接口进行广播
- rreq_process
  * 如果这个 RREQ 是自己发出去的就丢弃
  * 如果上一跳地址在黑名单中就丢弃
  * 如果这个 RREQ 已经来过（源节点地址和 rreq_id 在 record 中）就丢弃
  * 如果不属于上述三种情况，把这个 RREQ 插入 record
  * 处理一些 ext 信息
  * 然后是建立/更新反向路由的一些操作（查路由表，没有就插入，有就更新）
  * 如果自己就是目的节点，首先更新自己的 seqno（自己的比 RREQ 里的小，就变为 RREQ 里的，相等就把自己加一，大就啥也不干，其实就是为了保持序列号的单一性），然后生成 RREP 后调用 rrep_send
  * 如果自己是中间节点，那么检查路由表里面是否有目的节点的路由表项，如果有并且相应的序列号大于等于 RREQ 里面的序列号（说明足够新鲜），那么也可以生成 RREP 然后直接返回（这里假如 RREQ 的 g 有标记，还必须免费生成一个到目标节点的 RREP 并且发送），否则调用 rreq_forward 同时 ttl 减一（这里如果 ttl 等于 1 了就不再 forward 了，只有 ttl>1 才进行 forward，此外也要更新 seqno，如果 RREQ 的目的节点序列号比自己路由表中目的节点表项的序列号小，要更新 RREQ 目的节点序列号）。
- rreq_route_discovery
  * 寻路的函数
  * 查找 seek_list，存在则忽略本次寻路
  * 查找路由表是否已经存在目的节点信息，有的话可以复用一些信息（seqno、hcnt 等）
  * 调用 rreq_send，并且把当前寻路信息加入 seek_list
  * 设置 seek_timer 的 timeout（这里的 ttl 以及 timeout 要考虑是否设置了 expanding_ring_search，这在之前已经讨论过）
- rreq_local_repair
  * 和上一个基本一样，除了重新设置了 rt_timer 的 handler 函数以及对 timeout 和 ttl 的设置不太一样
- rreq_record_insert
  * 如果这个 RREQ 消息已经被缓存了，直接返回
  * 如果没有，生成并返回一个 rreq_record 结构体，这个结构体要加入 rreq_records 链表中
- . . .

在 rreq_process 函数参数中有两个参数是 ip_src 和 ip_dst，ip_src 记录的是上一跳节点的地址，ip_dst 好像没啥用。

***

### aodv_rrep.{h, c}

定义了 RREP 消息格式：

- type：跟 RREQ 一样，是为了适应 AODV_msg 而设置的；
- r, a：每个占一位，标识 RREP_REPAIR、RREP_ACK
- prefix：暂时不知道拿来做啥的
- res1, res2：保留位
- hcnt：跳数
- dest_addr：目标节点地址
- dest_seqno：目标节点序列号
- orig_addr：源节点地址
- orig_addr：源节点序列号（注意这里的源节点和目标节点不是基于发送 RREP 的，而是基于发送 RREQ 的，也就是说这个 RREP 消息会从目标节点地址发往源节点地址）
- lifetime：生存时间

![img](https://dragonylee-blog.oss-cn-beijing.aliyuncs.com/markdown/aodvuu_rrep_1.jpg)

RREP 消息格式

定义了 RREP_ack 消息的格式：type 和 reserved。RREP_ack 消息是当发送的 RREP 中的 a 位被置为 1 时，接受端就要返回这个消息表示已经收到 RREP 消息，这是为了防止单向路由导致的寻路失败（之前已经讨论过）。

定义了如下若干函数：

- rrep_create
  * 传入参数然后返回一个 RREP 结构体
- rrep_ack_create
  * 传入参数返回一个 RREP_ack 结构体（这里说明一下这里返回的结构体是一个指针（包括之前的 RREP 和 RREQ 也是），他们都是调用了 aodv_socket_new_msg，得到了 send_buf 的地址，然后直接在那个地址上操作返回）
- rrep_ack_process
  * 直接把路由表项中 ack_timer 从队列中移除（表示我已经收到了 ack，不用继续计时了）
- rrep_add_ext
  * 关于 ext 的不讨论
- rrep_send
  * 检查是否需要别人返回 RREP_ack 信息
  * 假设检测到是单向路由，那么需要 RREP_ack 信息，并且要把邻居节点断开（其实这里我自己都还不是很清楚……）然后设置 ack_timer
  * 调用 aodv_socket_send 进行发送
  * 添加 precursor 信息（表示有人要通过我去往某个目的节点）
- rrep_forward
  * 中间节点传递 RREP
  * 调用 aodv_socket_send 进行发送
  * 添加 precursor 信息
  * 更新反向路由 rev_rt 的 timeout
- rrep_process
  * RREP 处理函数
  * 找源节点和目的节点的路由表项 rev_rt 和 fwd_rt，如果源节点路由表项 rev_rt 不存在还要插入一个
  * 检测是否有 a 标志，有的话还得发送一个 RREP_ack，然后移除 a 标志（也就是说这个 a 标志是一次性的，需要就请求）
  * 检查 RREP 的源节点是不是就是自己，如果是的话还需要判断该 RREP 是不是路径修复返回的信息（也就是是不是我发起了路径修复，然后得到了一个 RREP 消息），如果是并且判断修复失败还要广播带有 RERR_NODELETE 的 RRER 消息，如果 RREP 的源节点不是自己，那么调用 rrep_forward 同时 ttl 减一
- rrep_add_hello_ext

注意 RREP 的 send、forward 等等操作传进去的参数和 RREQ 很不一样，为两个路由表项指针 rev_rt 和 fwd_rt，分别代表反向路由和正向路由，RREP 消息是由正向路由发过来，即将发往反向路由的。在其中更新 hcnt 时，只需要更新为正向路由 fwd_rt 的 hcnt 即可（到正向路由目的节点的跳数就是这个 RREP 走过的跳数）。

***

### aodv_rerr.{h, c}

RERR 消息是当一个节点检测到活跃路由的下一跳断链、路由修复失败、发包时发现没有活跃路由或者从邻居接收到 RERR 消息时才可能进行 RERR 消息的发送，并且发送 RERR 消息有单播（unicast）和广播（broadcast）两种形式，分别对应涉及到的前驱节点数目是否等于 1 。这两个文件定义了 RERR 消息的格式：

- type：类型，应当为 AODV_RERR，是为了适应 AODV_msg 消息格式
- n：占 1 位，用来标识 RERR_NODELETE（这个域的作用是在做路由修复的时候，让上游节点先不要删除路由）
- res1, res2：保留位
- dest_count：不可到达的目标节点数目（大于等于 1）
- dest_addr：不可到达的目标节点的地址
- dest_seqno：不可到达的目标节点的序列号

![img](https://dragonylee-blog.oss-cn-beijing.aliyuncs.com/markdown/aodvuu_rerr_1.jpg)

RERR 消息格式

原来手册中还给 RERR 消息定义了附带的（addr，seqno），在代码中体现为另一个结构体 RERR_udest（dest_addr，dest_seqno）。这里顺便提一句，之前讨论 aodv_socket 的时候，里面有两个缓冲区 send_buf 和 recv_buf，它们的最大长度 AODV_MSG_MAX_SIZE 就等于 RERR_SIZE + 100 * RERR_UDEST_SIZE，也就是说最多允许一个 RERR 消息携带一百个 udest。

定义了若干个与 RERR 相关的函数：

- rerr_create
  * 传入参数，生成并返回一个 RERR 结构体
- rerr_add_udest
  * 找到 RRER 消息大小后面那个地址，然后把那一块空间添加 udest 信息，然后把 RERR 消息的 dest_count 加一
- rerr_process
  * 先遍历 RERR 消息中的每个 udest
  * 对于每个 udest（都有一个不可达的目的节点地址和序列号）中的目的节点地址，找到自己路由表中对应的表项 rt（这里说明一下，当一个节点收到 RERR 消息需要处理的时候，可以获得一个上一跳节点地址 ip_src，既然这个 RERR 消息是从这里发过来的，那就说明对于 src 这个节点这所有的 udest 都是它不可达的，所以我就要检查路由表项，如果目的节点是 udest 的目的节点，并且下一跳的地址是这个 ip_src，那就说明现在对于我这个目的节点也不可达了）
  * 如果 rt 有效并且下一跳是 ip_src，那么：首先有一个 seqno 的检查（源码中没有启用），然后如果 n 位没有标识则 invalidate 该路由表项，然后要更新 rt 的 dest_seqno 为该 udest 的 seqno，然后如果该 rt 没有标识路径修复并且有前驱节点，那么还要把当前不可达的目的节点信息加入新的（也就是即将要转发的）RERR 消息里面（以上的一堆讨论都是针对同一个 udest 的，然后对所有 udest 都要做上面的事情）。
  * 最后如果新的 RERR 有东西的话，再进行 RERR 的发送，这里在之前遍历每个 udest 的时候也会动态维护单播和广播的信息。