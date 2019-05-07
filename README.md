<h2>Config</h2>
<ul>
<li>Get API yandex turbo OAuth token</li>
  <li>Create application on yandex https://oauth.yandex.ru/client/new
<img src="img/reg.png"/>
  </li>
<li>Edit admin/config/services/csas-ayt -> Host name</li>
<li>Enable admin/config/services/csas-ayt -> API yandex turbo debug</li>
<li>For get url once submit admin/config/services/csas-ayt -> Get_url_add_rss_chanel</li>
<li>For send submit admin/config/services/csas-ayt -> Upload turbo</li>
<li>For check processing submit admin/config/services/csas-ayt -> Task info total</li>
<li>For details task status info submit admin/config/services/csas-ayt -> Task info</li>
<li>When test success on https://webmaster.yandex.ru/site/https:mySite.ru:443/turbo/sources/ 
disable admin/config/services/csas-ayt -> API yandex turbo debug</li>
<li>Once send admin/config/services/csas-ayt -> Upload turbo batch</li>
<li>Check admin/reports/dblog</li>
<li>Check status admin/config/services/csas-ayt -> Task info total</li>
<li>Check complete & error admin/config/services/csas-ayt -> Task info</li>
<li>Finish after some time check https://webmaster.yandex.ru/site/https:mySite.ru:443/turbo/sources/</li>
  </ul>
P.S. For happy process use cron admin/config/services/csas-ayt -> Cron :)  
