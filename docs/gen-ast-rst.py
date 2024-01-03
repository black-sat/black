#!env python3

import jinja2
import json
import sys 
import click

@click.command()
@click.argument('data')
@click.argument('template')
def main(data: str, template: str):
    with open(data) as f:
        parsed = json.load(f)

    env = jinja2.Environment(loader=jinja2.FileSystemLoader('.'))
    print(env.get_template(template).render(parsed))

if __name__ == '__main__':
    exit(main())