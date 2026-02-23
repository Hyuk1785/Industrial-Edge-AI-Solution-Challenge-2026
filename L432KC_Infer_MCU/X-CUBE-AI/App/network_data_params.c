/**
  ******************************************************************************
  * @file    network_data_params.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-02-23T23:08:10+0900
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#include "network_data_params.h"


/**  Activations Section  ****************************************************/
ai_handle g_network_activations_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(NULL),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};




/**  Weights Section  ********************************************************/
AI_ALIGNED(32)
const ai_u64 s_network_weights_array_u64[481] = {
  0xbe653a053c946123U, 0x3e938afbbe8dbbc1U, 0xbe13738e3e13d4e8U, 0x3ec74270be9b990fU,
  0x3eb909b8bddd405aU, 0xbdc6b5aebd359406U, 0xbe652d9ebe2fbbaaU, 0x3dca00633d26941dU,
  0x3e84d2d53e01311aU, 0xbc68a43d3da019e6U, 0x3e2480663e89fd4cU, 0xbdb0f3723e1fec75U,
  0x3d8d0fdbbe97a249U, 0x3eb9c62cbec51f97U, 0x3e7faeb2beda655dU, 0x3cf55f30bd3dc1ddU,
  0x3e0667443da85abcU, 0xbdae36b43d9e6a69U, 0xbe3011bfbee0ff34U, 0x3e015c673ca8cb6cU,
  0x3ec0d456be17cdcaU, 0x3e8271963ed6a67fU, 0x3ec58b16bb98bbf0U, 0xbebbf9d8be4555e6U,
  0x3ce5dc263dd0a268U, 0x3eb2a7c8be87c424U, 0xbea8b07ebe1ef1cbU, 0xbe81d5523c0635efU,
  0xbe4d3a08be85573dU, 0xbd9c851d3e9246b1U, 0x3dfe9a893ea46748U, 0xbe39cc25be8fefa5U,
  0xbebea85cbe06d897U, 0xbdc9a21dbc863051U, 0xbec4206fbeba50d3U, 0x3dd20f27bdbdf676U,
  0x3e79a1403edf64e9U, 0xbe49ab493dacf9a9U, 0x3d6ab8e0be8848a8U, 0x3e8063603cc83f60U,
  0xbeb47358bebfaaf0U, 0x3d00db6dbe2761a1U, 0x3e8c42843df30b86U, 0xbe9dc32fbdc35ab2U,
  0x3e1fccae39db370dU, 0xbcba27e63dca3f0bU, 0x3eb620bc3e1fe13eU, 0x3e3dc8c33e416ef0U,
  0x38251e58bd487807U, 0x3ec3cff5be3be388U, 0x3e3d2276beb0cce3U, 0xbeb165723e4ccc3cU,
  0xbe4d96cbbdb70d5cU, 0x3e56d3913db2a9cdU, 0x3ccac5dabef822e5U, 0x3ecc94d2be63955cU,
  0xbec2f28cbe727d5aU, 0x3e9fc3c33e8ce6b2U, 0xbe0cd23f3b9f0b87U, 0x3ebaed1b3e3faf84U,
  0x3d75e880bd82e103U, 0xbec7f8f43e468475U, 0x3e9d1059be4b5d4cU, 0x3e05cc623e94a51fU,
  0x3ea43bbabeb76a7eU, 0x3e3a9482beabe7bcU, 0x3e8a7f7e3eaa4139U, 0x3eb0c3c43e1252ebU,
  0x3e63f1c73eb9c87bU, 0xbe181995bea47340U, 0xbe129b903eb4408eU, 0xbe8a8863be5b6a15U,
  0x3e86b3f8be4c9719U, 0xbd0cef16be65fae9U, 0x3cd1a99b3e0b7b7eU, 0x3e134990bda31d96U,
  0x3e962615be992ee8U, 0x3e96f1633eb661eeU, 0x3df0c1b53ea7910fU, 0xbe91a50b3e859371U,
  0xbea588123cf41a79U, 0x3cc93a0fbc28e66aU, 0x3ea902653be4f8dcU, 0xbd776e5c3e923063U,
  0x3ed812d03e4c3ad7U, 0x3e9fc6debe11427aU, 0x3e87a0073dcb0e0aU, 0xbdfb52c5bdd287e6U,
  0x3e7161553e3896edU, 0x3cf5dfc9bdade8caU, 0x3eb5b3f53e733abdU, 0x3e47da463ea1d25fU,
  0xbe8828f53eb80e21U, 0xbe95807abd7093acU, 0xbeaca407bea27443U, 0x3e538bdb3ea4072cU,
  0x3e83b5ebbeb7395fU, 0x3ec99309be81da61U, 0x3e899bc93cd8e948U, 0xbe191eb53e72d207U,
  0x3eee16c93e86ec46U, 0xbe1704d63bf8c396U, 0x3eabe52e3ead9424U, 0xbec97b153ef01d62U,
  0x3c8dc1903e9d69efU, 0x3e5617503e6fe07aU, 0x3ebb2f153e45cd8dU, 0x3eba1a523ea1d7f8U,
  0x3ef52a2fbe121f80U, 0x3dd0a4aabe2c54d6U, 0x3ed538953ed056afU, 0x3d3e0f2d3e29c165U,
  0xbe473640be2d5c61U, 0x3e3540aa3dfdcde8U, 0x3df6d96dbee4f0bfU, 0xbe94b951bec72f59U,
  0x3e5137d13e6bf0f2U, 0x3e86d3da3dddcaa6U, 0xbe8ccd8d3cbd8f58U, 0x3ea1ec6a3e1f2790U,
  0x3e95e0583e65463cU, 0xbdeac4aebe46b421U, 0x3d8f06cb3e4e4034U, 0x3eaa56ef3e9b0a24U,
  0x3e4a7f513d1fec09U, 0xbe2e1293be23e932U, 0xbe7ef1bbbebf4d60U, 0x3e1b373abea02b14U,
  0x3ca935c03dc37b11U, 0x3d6add07bdb50578U, 0x3d9153c4bb98e39eU, 0x3d08962b3dc3d7feU,
  0x3d0d7d613cac464aU, 0x3d27f0a33b07140fU, 0x3ddaf2123bf5553cU, 0x3dbdb3633cc07b0eU,
  0xbce0b3e23d1a6c5dU, 0x3d7af42ebd8c17f4U, 0x3c0ce85abc8ca7e0U, 0xbd6324393dacc96bU,
  0x3dd05ed53c006cfaU, 0x3dc3d5483b815e31U, 0xbd74cf273d86e092U, 0x3db760b13c052891U,
  0x3e84dfa0bd47fb05U, 0x3d4016653ee81271U, 0xbdfa3729be3b6dbfU, 0x3dfd031c3de94a60U,
  0x3e04ce26bde5698dU, 0x3eafd1f5be213e03U, 0xbe523df0be4b0a40U, 0x3ee73a1abe8b47e9U,
  0x3eaedaae3e87d22dU, 0xbcf327a43d0b88d6U, 0xbdc6c4cfbe51cd21U, 0x3e4b4ee33e586e4cU,
  0x3d7018aa3e92eda2U, 0x3e2feada3d9f3268U, 0xbe03e920be76a909U, 0xbcf89b843eb892a2U,
  0x3eaedb81beb91126U, 0xbe28fdd5bce3d71bU, 0x3e73842ebe8132fcU, 0xbe70d058be0cd0b0U,
  0x3d8c67173df4da36U, 0x3e3e027bbe73c916U, 0xbdde26923db28949U, 0xbdce7af9be22f924U,
  0xbe7957c2bd3e013bU, 0xbe8f73e4be10aea8U, 0x3e18758b3eac4c80U, 0xbd816f5bbcc2b3b2U,
  0x3dbedb523c1adc51U, 0xbe959d17bea3534eU, 0xbe9e5e9bbe157380U, 0x3e8bff423ceabeb4U,
  0x3a4f88c83d400222U, 0xb94611ee3ddf097eU, 0xbebe9d403e667d26U, 0xbe83523d3e23c8c4U,
  0xbe27c4b53cdbb77bU, 0x3e5d518cbe3d77b6U, 0xbdfbaeea3cdb5567U, 0xbe811c45be40e776U,
  0xbdab2b87bead9078U, 0xbe431f69be380c64U, 0x3d3d83cc3e0f4e1dU, 0x3e84e2643e5e377eU,
  0xbcd628a2bd89c445U, 0x3ddb11033e72dfcbU, 0x3da8b98d3e0fa1eeU, 0x3db44a24be8fd5b6U,
  0x3ece26cd3eacae43U, 0xbd3894a13debe127U, 0x3cf62df2bcc9766aU, 0x3e89bfd73e54f107U,
  0x3d98f0a33ec50bfdU, 0xbe69e5813ea7aff1U, 0x3e74653d3e946b06U, 0x3e2d56f0be700657U,
  0xbe3240f93e4ed494U, 0xbda5dbccbe0396b7U, 0x3d8052ed3d39626aU, 0x3e8d5a753e5ae0a4U,
  0xbe73e1033d8c1415U, 0xbb88ada0be488b25U, 0xbe4935c1bc2952dfU, 0x3d803e74bea9a7aaU,
  0x3e33a2dcbe345743U, 0xbe953e62be36ff45U, 0x3e9b72d13e4e4c38U, 0x3d2614aabd185034U,
  0xbe543686beacbeb5U, 0xbe906037bd01affaU, 0xbd522353bea8a84aU, 0x3e8c819dbe300032U,
  0xbea55d1b3db526a3U, 0xbea7264f3e8caaa0U, 0xbc132c5d3e9c4b5eU, 0x3e4134c4be7343f3U,
  0xbd30ca933de92763U, 0x3dd4fe7bbe0c74e7U, 0xbeaf1fd8be1b145eU, 0x3e9da812be7310f9U,
  0xbe142ada3e06a2e2U, 0x3d9858a5be2da71dU, 0x3e9f806cbe4972f1U, 0xbe0d23123ed6375aU,
  0x3e03d001be4af591U, 0xbe4f7f8cbe89097cU, 0x3ebace063e2930d1U, 0xbe6e8c1a3d88548fU,
  0xbe3f53ce3d37345eU, 0xbe9c9e99be8cba04U, 0x3ce1fded3df4320fU, 0xbe36001ebd1d6603U,
  0xbd803ad4be0afb60U, 0xbdd511e13e65ef9eU, 0xbd6a6beabb07a858U, 0xbacb278b3c202b8cU,
  0x3da02ab83db22a82U, 0xbe42ec27be9da70fU, 0xbe2897acbab1ebacU, 0x3d357b32bd6a90d5U,
  0xbe110ccbbd15e7c1U, 0xbdf9aedcbe4154f5U, 0xbd8141b4beb884ecU, 0xbeaf749c3e0a77aeU,
  0x3e61e9e63e4f91a5U, 0x3ea1f1343e901b7aU, 0x3ea1c6bbbe4f16aeU, 0xbb77f6bf3e1a15caU,
  0x3d1ce194be0d100dU, 0x3e172b233e52d693U, 0x3e6d885b3d989f79U, 0xbe3f0b943eacafafU,
  0x3e8ba416be56479dU, 0xbd8c6a27bd950cb0U, 0xbe2f46a6bd978b2dU, 0xbe22de95bdb6a022U,
  0x3e3ab396bdf3764aU, 0x3e488d2a3db6321dU, 0xbd7143eebe46d932U, 0x3e958f62bd8da991U,
  0x3e6b51c73e00eeb0U, 0x3e68d817be9dd3b4U, 0x3e83ffc73ea6e3deU, 0x3e62680a3e14ded2U,
  0x3e8922dbbdf6d931U, 0x3ea70d4cbd8fd7b0U, 0x3caf95ba3e7a784cU, 0xbebdb8b7be4192d7U,
  0x3e87e245bdc33de4U, 0xbce4be0fbe568d04U, 0x3e982577be39fbe5U, 0x3e86fd17be501fa3U,
  0x3dd513123e2c675bU, 0x3e94a67fbe7c8f05U, 0xbdb04b513c9d6acaU, 0x3e5998f0bd3bcba6U,
  0xbd893ed0bd91085cU, 0x3e7674c8be756b16U, 0x3e5dd1b93da95c9dU, 0xbe5618ea3ebc4f1fU,
  0x3e410fa3bdab7362U, 0x3eb1f0513d9de531U, 0x3c660ba5be0a7452U, 0xbea721e3bdcc2583U,
  0x3e82162c3e85bfffU, 0xbe598c0b3e1b74d9U, 0xbd8799943e07bd70U, 0xbd28f092be189bc5U,
  0xbe9e44f9bde8e65dU, 0x3e2dde743e930a27U, 0xbe845f90be8e0274U, 0x3d058431bec6c85dU,
  0x3d7e5b793e43b3ffU, 0x3ec5cd96bdb113bbU, 0xbca575b7be84ee7eU, 0xbec844493e9e49ccU,
  0x3e7eb05b3cfd6652U, 0xbe561741be86d6ffU, 0xbe29be70be9c18adU, 0x3e77cb993e38e248U,
  0x3e178c583d361129U, 0xbe9e352cbe126945U, 0x3eb404d7be375e3fU, 0xbe83375a3e513a11U,
  0x3eba516e3e6725aeU, 0xbe238a43bb9e98cfU, 0xbda851ecbe9e0804U, 0x3ecf5212bead7d2eU,
  0xbe886b783e05e6a4U, 0xbe45d4523c85c03eU, 0x3e877ee53d7e57daU, 0x3e7d41073e6ed26aU,
  0x3e942e2f3e6738d6U, 0x3e0932063e9fee71U, 0xbe4eda773e78e4dfU, 0x3e44e9de3e0c513cU,
  0xbe3c838bbe6e6958U, 0xbe8676d3bdeb6691U, 0x3e7bddb13d9b22f6U, 0x3eb4a3d13d818d80U,
  0x3e5b60e6bd61a112U, 0x3e999504bd883b1aU, 0x3e5fd59dbe81d080U, 0x3e8e7ff63d904dd0U,
  0xbdafca2b3d85a658U, 0xbe3e521bbdd27dccU, 0x3e0f4f2e3da0cecbU, 0x3e7ba213be1c631eU,
  0x3dcfa695be88e510U, 0xbd5efe483e8baedaU, 0x3d60c6f33e5c1132U, 0xbec49495be12c405U,
  0xbeaaae133ce2fab6U, 0x3e656c763e2dfaedU, 0xbe48d9e8bc9cb68fU, 0x3dc5cc173cd3ec48U,
  0x3c8a08833e16f9f3U, 0x3da06abfbe3436d8U, 0xbd89c99cbae0bb20U, 0xbeb44bf83e7ae214U,
  0xbe539c843e9487bfU, 0xbe98cb923d74c1cdU, 0xbdb11039be7a3764U, 0xbe3c7855bd628516U,
  0xbe7ee1b8bde32325U, 0xbe0f12b93e3e45c2U, 0xbc7e97983e81d23fU, 0xbe18b5323e81c1e9U,
  0xbe7c3367bc8347acU, 0x3ecbb060be659ed2U, 0xbdd4034b3dc2d2ceU, 0x3e6e32593e1dff9eU,
  0xbe986410bdb9a05aU, 0xbe8bfac83e77f6d9U, 0x3e862ae8be0ead4aU, 0x3d488d0b3ea4b7b9U,
  0xbdc4c9233e8cce94U, 0xbe4c3e65be6d4f95U, 0xbdc3b19fbe209194U, 0xbd18077bbe7c2a33U,
  0x3e1d9ad43e3295f8U, 0xbdfae6cc3cdb537eU, 0x3e8472313ea41f5aU, 0x3e81157bbe8b93f2U,
  0x3e976dab3ea2a684U, 0x3e1b461e3e8972c4U, 0x3e17c14c3ec1fa50U, 0xbe32c48b3ecf68bcU,
  0xbdfcb8e93eb87f0cU, 0xbe9736efbdd07aeeU, 0x3c1dbb27be572dfeU, 0xbd5460473e327c3dU,
  0xbe9a8e343e3fe490U, 0xbd3f1122be44be2eU, 0xbea0a05d3e7b5570U, 0xbe4379e4bea5ef6fU,
  0xbdac69a13e47a71eU, 0xbe73a280ba8296a7U, 0x3e81590e3eb682c1U, 0x3e9323afbeb38f02U,
  0x3de14b783ed8daa5U, 0xbd4b5b30bda8cef5U, 0x3de2674b3e23b0aeU, 0x3eac52aabc319233U,
  0x3e0f64a4be7b8d22U, 0xbe994f79bd5f170fU, 0x3e616cb7bcf6d311U, 0xbdabd30fbc21d868U,
  0xbdbb8f463de37490U, 0x3a1fedc13e071eadU, 0x3e827631bcd03534U, 0xbe7c55c8be484c94U,
  0xbe335b33be84ac86U, 0xbdce5ade3dacb344U, 0xbe7b88113e90527cU, 0x3ed6cec63d0ab844U,
  0x3bccc5003d931288U, 0x3daa73c3bccfb06bU, 0x3dbc39cc3cfcce43U, 0x3c896999bcfbc0e4U,
  0xbd172a203d45a7f3U, 0xbc56742c3ca9399eU, 0x3d9b1960bd7acc4bU, 0x3dd7a5973d80612fU,
  0x3e445fa13edf6184U, 0x3ec26311be61737fU, 0x3ecc1fd8be541becU, 0x3e23365d3e396713U,
  0x3ccdf2df3f14f196U, 0xbe8d43e03ed8f6fcU, 0xbda41817beaaa20cU, 0x3ec31847bef254dfU,
  0x3eaee764beb2b0b0U, 0x3f1d12aabe984a0fU, 0x3f01448abe801222U, 0x3e086884be90f1f9U,
  0xbdcf00593c755b5cU, 0x3d9461d1bec0f3c5U, 0x3efd2061be73932cU, 0x3d97a719be4f9f39U,
  0xbc17796a3ee548daU, 0x3e6b1fa03ea17601U, 0xbe1aacc9bf01364cU, 0x3e15100b3e56b8d6U,
  0x3ece3f97bec7d579U, 0x3e45208b3e3d09d4U, 0xbe6bc134bed0c5caU, 0xbaf723c8bdf6cc5eU,
  0x3ddd11cdbe90fe85U, 0x3f001ce93eb126adU, 0xbecb02e73f065ac1U, 0xbe92151a3e3a9a0aU,
  0xbe78f0debef6a05bU, 0x3e8d5d273dee789dU, 0xbe34ec573dbc4460U, 0x3e1cc5dd3efb5468U,
  0xbdd87410bb640cfdU, 0x3dc8f6bb3daf8762U, 0x3f10984f3e5c22efU, 0xbee334a0bd8be28eU,
  0xbe9c9b4abe98f75eU, 0xbeb9a9243ec72d03U, 0x3c90bd97bd988340U, 0x3f1862c33e9f4a78U,
  0x3e8004793e51747dU, 0xbeb79325bed01b95U, 0x3e8533d03ecb8828U, 0x3db25c0ebebcf9ddU,
  0xbe6cf48a3f109965U, 0x3e951b1f3dbcef29U, 0xbe84edd83e70a3d8U, 0xbe91842dbe8ad594U,
  0x3e3a6ccf3ea73297U, 0x3f0c2bd13de66afdU, 0x3d66b1a73e85784dU, 0xbeaf44903df1b7d1U,
  0x3d6ea827be36fd3eU, 0x3eb4e6cfbefe79f8U, 0xbe020e963e85d4d4U, 0xbea03ac9befdf100U,
  0x3dbe97f3bd185919U, 0xbd8e6678be88e275U, 0xbdc01e843e40cc90U, 0xbec3b4ba3e14831bU,
  0xbe9955673d2a0e5cU, 0x3ef71a9c3e1b8df8U, 0xbd67d1ffbd85f43cU, 0xbec62f33bedcb304U,
  0x3dde360b3cce397dU, 0x3dbb42573d68717cU, 0x3d4f6c513de00099U, 0x3b3186e2bd25f030U,
  0xbf1640083ee7beb6U, 0xbe94ad4d3ec83b55U, 0x3f48b665bf2c739dU, 0x3f13d3f5bee67a42U,
  0xbc2ac473U,
};


ai_handle g_network_weights_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(s_network_weights_array_u64),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};

