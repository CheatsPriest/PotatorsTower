import hashlib
from time import strptime

from flask import Flask, request, redirect, url_for, session
from flask import render_template, jsonify
from flask_wtf import FlaskForm
from wtforms import StringField, PasswordField, BooleanField, SubmitField, ValidationError,SelectField
from wtforms.validators import DataRequired, Email, Length, EqualTo
import os
import psycopg2
from datetime import datetime

def connect():
    return psycopg2.connect(dbname=os.environ['POSTGRES_DB'], user=os.environ['POSTGRES_USER'],
                            password=os.environ['POSTGRES_PASSWORD'], host=os.environ['POSTGRES_HOST'],
                            port=os.environ['POSTGRES_PORT'])
class Config:
    SECRET_KEY = os.environ.get('SECRET_KEY') or 'you-will-never-guess'


class LoginForm(FlaskForm):
    username = StringField('Username', validators=[DataRequired()])
    password = PasswordField('Password', validators=[DataRequired()])
    remember_me = BooleanField('Remember Me')
    submit = SubmitField('Sign In')

    def validate_username(self, username):
        conn = connect()
        cursor = conn.cursor()
        cursor.execute("SELECT login FROM users WHERE login = %s", (username.data,))
        user = cursor.fetchone()
        conn.close()

        if not user:
            raise ValidationError('Это имя пользователя не существует')

    def validate_password(self, password):
        conn = connect()
        cursor = conn.cursor()

        cursor.execute("SELECT password FROM users WHERE login = %s", (self.username.data,))
        user = cursor.fetchone()
        conn.close()
        if not user or not hash_password(password.data) == user[0]:
            raise ValidationError('пользователя не существует или неправильный пароль')


class NewLoginForm(FlaskForm):
    username = StringField('Username', validators=[DataRequired()])
    tg_nick = StringField('Логин телеграм')
    password = PasswordField('Password', validators=[DataRequired()])

    def validate_username(self, username):
        conn = connect()
        cursor = conn.cursor()
        cursor.execute("SELECT login FROM users WHERE login = %s", (username.data,))
        user = cursor.fetchone()
        conn.close()

        if not user:
            raise ValidationError('Это имя пользователя не существует')

    def validate_password(self, password):
        conn = connect()
        cursor = conn.cursor()
        cursor.execute("SELECT password FROM users WHERE login = %s", (self.username.data,))
        user = cursor.fetchone()
        conn.close()
        if not user or not hash_password(password.data) == user[0]:
            raise ValidationError('пользователя не существует или неправильный пароль')


class RegistrationForm(FlaskForm):
    username = StringField('Имя пользователя', validators=[DataRequired()])
    email = StringField('Email', validators=[ Email(message='Введите корректный email адрес')])
    tg_nick = StringField('Логин телеграм')
    password = PasswordField('Пароль', validators=[DataRequired(),
                                                   Length(min=6, message='Пароль должен содержать минимум 6 символов')])
    password2 = PasswordField('Повторите пароль',
                              validators=[DataRequired(), EqualTo('password', message='Пароли должны совпадать')])
    submit = SubmitField('Зарегистрироваться')

    def validate_email(self, email):
        conn = connect()
        cursor = conn.cursor()
        cursor.execute("SELECT email FROM users WHERE email = %s", (email.data,))
        user = cursor.fetchone()

        conn.close()
        if user:
            raise ValidationError('Эта почта уже занята')

    def validate_username(self, username):
        if len(username.data) <= 3:
            raise ValidationError("too short")
        conn = connect()
        cursor = conn.cursor()
        cursor.execute("SELECT login FROM users WHERE login = %s", (username.data,))
        user = cursor.fetchone()
        conn.close()

        if user:
            raise ValidationError('Это имя пользователя или почта уже занято')


class SearchForm(FlaskForm):
    site = StringField('', validators=[DataRequired(), ])
    submit = SubmitField('Искать')


class AddForm(FlaskForm):
    site = StringField('', validators=[DataRequired(), ])
    submit = SubmitField('Добавить')

class ContentForm(FlaskForm):
    content = StringField('set content', validators=[DataRequired(), ])
    set = SubmitField('Сохранить')

class ContentTypeForm(FlaskForm):
    type = StringField('set content type', validators=[DataRequired(), ])
    use = SubmitField('Сохранить')

class PathForm(FlaskForm):
    path = StringField('set content type', validators=[DataRequired(), ])
    pat = SubmitField('Сохранить')

class CodeForm(FlaskForm):
    code = StringField('set code', validators=[DataRequired(), ])
    cod = SubmitField('Сохранить')


class SubForm(FlaskForm):
    sub = SubmitField('Не подписан')
    sub.data = False

class HTTPForm(FlaskForm):
    htt = SubmitField('Change to https')
    htt.data = False

class PriorityForm(FlaskForm):
    pri = SelectField('Adjust priority', choices=[(4,'extreme'),(3,'high'),(2,'medium'),(1,'low')])
    pris = SubmitField('set')
    pris.data = False


class SSLForm(FlaskForm):
    sll = SubmitField('skip SSL')
    sll.data = False

class MediaForm(FlaskForm):
    med = SubmitField('skip SSL')
    med.data = False

class addForm(FlaskForm):
    add = SubmitField('➕ Добавить сайт')
    add.data = False


name = "1"
app = Flask(name)
app.config.from_object(Config)


def hash_password(password):
    return hashlib.sha256(password.encode('utf-8')).hexdigest()


def add_serv(servname):
    con_serv = connect()

    c = con_serv.cursor()
    c.execute("SELECT endpoint FROM new_servers WHERE endpoint = %s", (servname,))
    v = con_serv.cursor()
    v.execute("SELECT endpoint FROM servers WHERE endpoint = %s", (servname,))
    print(1)
    if not (c.fetchone() or v.fetchone()):
        print(2)
        c.execute("INSERT INTO new_servers(endpoint) values(%s)", (servname,))
    con_serv.commit()
    con_serv.close()


def add_sub(username, servname):
    con_serv = connect()
    c = con_serv.cursor()
    v = con_serv.cursor()
    c.execute("SELECT id FROM servers WHERE endpoint = %s", (servname,))
    v.execute("SELECT id FROM users WHERE login = %s", (username,))
    ca = c.fetchone()
    va = v.fetchone()
    print(ca, va)
    if ca and va:
        sid = int(ca[0])
        uid = int(va[0])
        c.execute("SELECT * FROM user_subscriptions WHERE user_id = %s AND site_id=%s", (uid, sid))
        print(1)
        if not c.fetchone():
            c.execute("INSERT INTO user_subscriptions(user_id, site_id) values(%s, %s)", (uid, sid))
            print(2)

        else:
            c.execute("DELETE FROM user_subscriptions WHERE user_id = %s AND site_id = %s", (uid, sid))
            print(3)

    con_serv.commit()
    con_serv.close()


@app.route('/profile', methods=['GET', 'POST'])
def profile():
    username = session['username']
    conn = connect()
    c = conn.cursor()
    c.execute("SELECT email, id, tg_tag FROM users WHERE login = %s", (username,))

    inf = c.fetchone()
    print(username)
    email = inf[0]
    id = inf[1]
    tg = inf[2]
    c.execute("SELECT COUNT(*) FROM user_subscriptions WHERE user_id = %s", (id,))
    d = c.fetchone()
    return render_template("user_profile.html", username=username, email=email, id=id, subscriptions_count=d[0],
                           tg_nickname=tg, user_authenticated=session.get('user_authenticated', False),
                           admin=session.get('admin', False))


@app.route('/add', methods=['GET', 'POST'])
def add():
    form = AddForm()
    if form.validate_on_submit():
        siteref = form.site.data.split("/")
        if ':' in siteref[0]:
            site = siteref[2]
        else:
            site = siteref[0]
        add_serv(site)
    # site - domen
    return render_template('add.html', form=form, user_authenticated=session.get('user_authenticated', False),
                           admin=session.get('admin', False))


@app.route('/', methods=['GET', 'POST'])
def index():
    form = SearchForm()
    if form.validate_on_submit():
        siteref = form.site.data.split("/")
        if siteref[0] == "&#129313;":
            if 'admin' not in session:
                session['admin'] = True
            else:
                session['admin'] = not session['admin']
            print(session['admin'])
            return redirect('/')
        if ':' in siteref[0]:
            site = siteref[2]
        else:
            site = siteref[0]
        return redirect('/site/' + site)
    # site - domen
    return render_template('index.html', form=form, user_authenticated=session.get('user_authenticated', False),
                           admin=session.get('admin', False))

@app.route('/site/<sitename>', methods=['GET', 'POST'])
def sites(sitename):
    if not session.get('user_authenticated'):
        return redirect(url_for('login'))
    form = SubForm()
    form2 = addForm()
    form3 = ContentForm()
    form4 = ContentTypeForm()
    formPri = PriorityForm()
    formmed = MediaForm()
    formpat = PathForm()

    formhtp = HTTPForm()
    formssl = SSLForm()
    conn = connect()
    c = conn.cursor()
    v = conn.cursor()

    c.execute("SELECT id FROM servers WHERE endpoint = %s", (sitename,))
    v.execute("SELECT id FROM users WHERE login = %s", (session['username'],))
    ca = c.fetchone()
    va = v.fetchone()
    ssl = ''
    htp = ''
    med = ''
    pth = ''
    ctn = ''
    ctn_type = ''
    priority = ''
    pings = []
    time = []
    delay = []
    if ca and va:
        sid = int(ca[0])
        uid = int(va[0])
        c.execute("SELECT * FROM user_subscriptions WHERE user_id = %s AND site_id=%s", (uid, sid))
        if (c.fetchone()):
            form.sub.label.text = "Подписан"
        else:
            form.sub.label.text = "Не подписан"

        c.execute(''' SELECT * FROM servers WHERE endpoint = %s LIMIT 1''', (sitename,))
        siteid = c.fetchone()
        time = []
        delay = []
        c.execute(
            ''' SELECT cheack_ssl, is_https, load_media, path, content, content_type,priority FROM servers WHERE endpoint = %s LIMIT 1''',
            (sitename,))
        a = c.fetchone()
        ssl = a[0]
        htp = a[1]
        med = a[2]
        pth = a[3]
        ctn = a[4]
        ctn_type = a[5]
        dic = {1: 'low', 2: 'medium', 3: 'high', 4: 'extreme'}
        priority = dic[a[6]]
        if (ssl):
            formssl.sll.label.text = "SSL used"
        else:
            formssl.sll.label.text = "SSL not used"
        if (htp):
            formhtp.htt.label.text = "https"
        else:
            formhtp.htt.label.text = "http"
        if (med):
            formmed.med.label.text = "load media"
        else:
            formmed.med.label.text = "do not load media"
        if siteid:
            c.execute(''' SELECT * FROM logs WHERE server_id = %s ORDER BY date_time DESC''', (siteid[0],))
        for row in c.fetchall():
            pings.append(
                {'status': row[3], 'response_time': row[4], 'timestamp': row[2], 'ans': row[5], "error": row[6]})

            time.append(str(row[2]))
            delay.append(row[4])
        time.reverse()
        delay.reverse()
        if request.method == 'POST':
            if formPri.pris.data:
                print(formPri.pri.data[0])
                c.execute("UPDATE servers SET priority = %s WHERE endpoint = %s", (formPri.pri.data[0], sitename))
            if formssl.sll.data:
                c.execute("UPDATE servers SET cheack_ssl = %s WHERE endpoint = %s", (not ssl, sitename))
            if formhtp.htt.data:
                c.execute("UPDATE servers SET is_https = %s WHERE endpoint = %s", (not htp, sitename))
            if form.sub.data:
                add_sub(session['username'], sitename)
            if formmed.med.data:
                c.execute("UPDATE servers SET load_media = %s WHERE endpoint = %s", (not med, sitename))
            if (form3.set.data):
                c.execute("UPDATE servers SET content = %s WHERE endpoint = %s", (form3.content.data, sitename))
            if (formpat.pat.data):
                c.execute("UPDATE servers SET path = %s WHERE endpoint = %s", (formpat.path.data, sitename))
            if (form4.use.data):
                print(form4.type.data)
                c.execute("UPDATE servers SET content_type = %s WHERE endpoint = %s", (form4.type.data, sitename))
            conn.commit()
            conn.close()
            return redirect('/site/' + sitename)

    return render_template('profile.html', pings=pings, ctn=ctn, pth=pth, ctn_type=ctn_type, formpat=formpat,
                           formssl=formssl, formmed=formmed, formhtp=formhtp, form=form, time=time, delay=delay,
                           form2=form2,
                           sitename=sitename, form3=form3, form4=form4, formpri=formPri, priority=priority,
                           user_authenticated=session.get('user_authenticated', False),
                           admin=session.get('admin', False))


@app.route('/subs', methods=['GET', 'POST'])
def subs():
    if not session.get('user_authenticated'):
        return redirect(url_for('login'))
    conn = connect()
    c = conn.cursor()
    v = conn.cursor()
    v.execute("SELECT id FROM users WHERE login = %s", (session["username"],))
    uid = v.fetchone()
    c.execute("SELECT site_id FROM user_subscriptions WHERE user_id = %s", (uid[0],))
    cid = c.fetchall()
    c.execute("SELECT * FROM servers WHERE id = ANY(%s)", (cid,))
    pings = []
    for row in c.fetchall():
        pings.append({'site_name': row[1], 'status': row[2], 'response_time': row[5], 'timestamp': row[3]})

    c.execute('SELECT COUNT(*) FROM servers WHERE id = ANY(%s)', (cid,))
    sites_count = c.fetchone()[0]

    c.execute('''SELECT COUNT(*) FROM servers WHERE status = 'up' AND  id = ANY(%s)''', (cid,))
    online_count = c.fetchone()[0]

    c.execute('''SELECT COUNT(*) FROM servers WHERE status = 'down' AND  id = ANY(%s)''', (cid,))
    offline_count = c.fetchone()[0]

    c.execute('''SELECT AVG(delay) FROM servers WHERE status = 'up' AND  id = ANY(%s)''', (cid,))
    avg_response = int(c.fetchone()[0] or 0)
    conn.close()

    return render_template('subs.html', pings=pings, sites_count=sites_count, online_count=online_count,
                           offline_count=offline_count, avg_response=avg_response,
                           user_authenticated=session.get('user_authenticated', False),
                           admin=session.get('admin', False))


@app.route('/login', methods=['GET', 'POST'])
def login():
    form = LoginForm()
    if form.validate_on_submit():
        session['user_authenticated'] = True
        session['admin'] = False
        session['username'] = form.username.data
        return redirect(url_for('index'))
    return render_template('login.html', form=form, user_authenticated=session.get('user_authenticated', False),
                           admin=session.get('admin', False))


@app.route('/new_login', methods=['GET', 'POST'])
def new_login():
    form = NewLoginForm()
    if form.validate_on_submit():
        tg = ""
        if form.tg_nick.data:
            tg = form.tg_nick.data
        session['user_authenticated'] = True
        session['username'] = form.username.data
        con = connect()
        cur = con.cursor()
        cur.execute('INSERT INTO users (login, email, password, tg_tag) VALUES (%s, %s, %s, %s)',
                    (form.username.data, form.email.data, hash_password(form.password.data), tg))
        con.commit()
        con.close()
        return redirect(url_for('index'))
    return render_template('new_login.html', form=form, user_authenticated=session.get('user_authenticated', False),
                           admin=session.get('admin', False))


@app.route('/register', methods=['GET', 'POST'])
def register():
    form = RegistrationForm()
    if form.validate_on_submit():
        session['admin'] = False
        tg = ""
        if form.tg_nick.data:
            tg = form.tg_nick.data
        session['user_authenticated'] = True
        session['username'] = form.username.data
        con = connect()
        cur = con.cursor()
        cur.execute('INSERT INTO users (login, email, password, tg_tag) VALUES (%s, %s, %s, %s)',
                    (form.username.data, form.email.data, hash_password(form.password.data), tg))
        con.commit()
        con.close()
        return redirect(url_for('index'))
    return render_template('register.html', form=form, user_authenticated=session.get('user_authenticated', False),
                           admin=session.get('admin', False))


@app.route('/logout')
def logout():
    session.pop('user_authenticated', None)
    session.pop('username', None)
    return redirect(url_for('index'))


@app.route('/monitoring', methods=['GET', 'POST'])
def monitoring():
    if not session.get('user_authenticated'):
        return redirect(url_for('login'))


    conn = connect()
    c = conn.cursor()
    c.execute(''' SELECT * FROM servers''')


    pings = []
    for row in c.fetchall():
        pings.append({'site_name': row[1], 'status': row[2], 'response_time': row[4], 'timestamp': row[3]})


    c.execute('SELECT COUNT(*) FROM servers')
    sites_count = c.fetchone()[0]

    c.execute('''SELECT COUNT(*) FROM servers WHERE status = 'up' ''')
    online_count = c.fetchone()[0]

    c.execute('''SELECT COUNT(*) FROM servers WHERE status = 'down' ''')
    offline_count = c.fetchone()[0]

    c.execute('''SELECT AVG(delay) FROM servers WHERE status = 'up' ''')
    avg_response = int(c.fetchone()[0] or 0)


    conn.close()

    return render_template('table.html', pings=pings, sites_count=sites_count, online_count=online_count,
                           offline_count=offline_count, avg_response=avg_response,
                           user_admin=session.get('admin', False),
                           user_authenticated=session.get('user_authenticated', False),
                           admin=session.get('admin', False))

#Rest-Api starts from here

@app.route('/data/table',methods=['GET'])
def data_table():
    conn = connect()
    c = conn.cursor()

    c.execute(''' SELECT * FROM servers''')

    servers = []
    for row in c.fetchall():
        servers.append({'site_name': row[1], 'status': row[2], 'response_time': row[4], 'timestamp': row[3]})

    c.execute('SELECT COUNT(*) FROM servers')
    sites_count = c.fetchone()[0]

    c.execute('''SELECT COUNT(*) FROM servers WHERE status = 'up' ''')
    online_count = c.fetchone()[0]

    c.execute('''SELECT COUNT(*) FROM servers WHERE status = 'down' ''')
    offline_count = c.fetchone()[0]

    c.execute('''SELECT AVG(delay) FROM servers WHERE status = 'up' ''')
    avg_response = int(c.fetchone()[0] or 0)
    conn.close()

    return jsonify(servers=servers, sites_count=sites_count, online_count=online_count,
                           offline_count=offline_count, avg_response=avg_response,)

@app.route('/data/site/<sitename>/status',methods=['GET'])
def data_site_table(sitename):
    conn = connect()
    c = conn.cursor()
    pings = []
    c.execute(''' SELECT * FROM servers WHERE endpoint = %s LIMIT 1''', (sitename,))
    siteid = c.fetchone()

    if siteid:
        c.execute(''' SELECT * FROM logs WHERE server_id = %s ORDER BY date_time DESC''', (siteid[0],))
    else:
        return jsonify({'error': 'No such site'})
    for row in c.fetchall():
        pings.append({'status': row[3], 'response_time': row[4], 'timestamp': row[2], 'ans': row[5], "error": row[6]})
    conn.close()
    return jsonify(pings)

@app.route('/data/server/<sid>',methods=['GET'])
def data_id_table(sid):
    conn = connect()
    c = conn.cursor()
    pings = []
    c.execute(''' SELECT * FROM servers WHERE id = %s LIMIT 1''', (sid,))
    siteid = c.fetchone()

    if siteid:
        c.execute(''' SELECT * FROM logs WHERE server_id = %s ORDER BY date_time DESC''', (sid,))
    else:
        return jsonify({'error': 'No such site'})
    for row in c.fetchall():
        pings.append({'status': row[3], 'response_time': row[4], 'timestamp': row[2], 'ans': row[5], "error": row[6]})
    conn.close()
    return jsonify(pings)

@app.route('/data/<username>/subs',methods=['GET'])
def data_subs_table(username):
    conn = connect()
    c = conn.cursor()
    v = conn.cursor()
    v.execute("SELECT id FROM users WHERE login = %s", (username,))
    uid = v.fetchone()
    if not uid:
        return jsonify({'error': 'No such user'})
    c.execute("SELECT site_id FROM user_subscriptions WHERE user_id = %s", (uid[0],))
    cid = c.fetchall()
    c.execute("SELECT * FROM servers WHERE id = ANY(%s)", (cid,))
    pings = []
    for row in c.fetchall():
        pings.append({'site_name': row[1], 'status': row[2], 'response_time': row[5], 'timestamp': row[3]})

    c.execute('SELECT COUNT(*) FROM servers WHERE id = ANY(%s)', (cid,))
    sites_count = c.fetchone()[0]

    c.execute('''SELECT COUNT(*) FROM servers WHERE status = 'up' AND  id = ANY(%s)''', (cid,))
    online_count = c.fetchone()[0]

    c.execute('''SELECT COUNT(*) FROM servers WHERE status = 'down' AND  id = ANY(%s)''', (cid,))
    offline_count = c.fetchone()[0]

    c.execute('''SELECT AVG(delay) FROM servers WHERE status = 'up' AND  id = ANY(%s)''', (cid,))
    avg_response = int(c.fetchone()[0] or 0)
    conn.close()

    return jsonify(servers=pings, sites_count=sites_count, online_count=online_count,
                   offline_count=offline_count, avg_response=avg_response )



@app.route('/data/server/<id>/filters',methods=['GET'])
def data_serve_filtered_table(id):
    filters = dict()
    filters['after'] = request.args.get('after', None)
    if filters['after']:
        filters['after'] = datetime.strptime(filters['after'],"%Y-%m-%d_%H:%M:%S")
    filters['num'] = request.args.get('num', None)
    filters['offset'] = request.args.get('offset', None)
    filters['before'] = request.args.get('before', None)
    if filters['before']:
        filters['before'] = datetime.strptime(filters['before'],"%Y-%m-%d_%H:%M:%S")
    conn = connect()
    c = conn.cursor()
    values = [int(id)]
    req =''' SELECT * FROM logs WHERE server_id = %s '''
    if filters['after']:
        req = req + 'AND date_time > %s'
        values.append(filters['after'])
    if filters['before']:
        req = req + 'AND date_time < %s'
        values.append(filters['before'])
    if filters['num']:
        req = req + ' LIMIT (%s)'
        values.append(filters['num'])
    if filters['offset']:
        req = req + ' OFFSET (%s)'
        values.append(filters['offset'])


    print(req, tuple(values)+())
    c.execute(req, vars=tuple(values))
    servers = []
    for row in c.fetchall():
        servers.append({'site_name': row[1], 'status': row[2], 'response_time': row[4], 'timestamp': row[3]})

    return jsonify(servers=servers)


if __name__ == '__main__':
    app.run(debug=True)